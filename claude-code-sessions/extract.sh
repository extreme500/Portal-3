#!/usr/bin/env bash
# =============================================================================
# extract_claude_history.sh
# Extrai conversas do Claude Code de um repositório específico.
# Gera: (1) arquivos JSONL brutos  (2) transcrições em texto legível
#
# Uso:
#   ./extract_claude_history.sh /caminho/para/seu/repositorio
#   ./extract_claude_history.sh /caminho/para/seu/repositorio ./saida
# =============================================================================

set -euo pipefail

# ---------------------------------------------------------------------------
# Parâmetros
# ---------------------------------------------------------------------------
REPO_PATH="${1:-.}"
OUTPUT_DIR="${2:-./claude_history_export}"

# Resolve o caminho absoluto do repositório ("." vira o pwd atual)
REPO_PATH="$(cd "$REPO_PATH" && pwd)"

# ---------------------------------------------------------------------------
# Localizar a pasta do projeto no ~/.claude/projects/
# O Claude Code transforma o caminho absoluto em nome de diretório
# substituindo "/" por "-" (e removendo a "/" inicial).
# Ex.: /Users/foo/meu-repo  →  -Users-foo-meu-repo
# ---------------------------------------------------------------------------
CLAUDE_PROJECTS="${CLAUDE_CONFIG_DIR:-$HOME/.claude}/projects"

# Gera o sufixo de busca (qualquer separador pode ter sido usado)
PROJECT_SUFFIX="${REPO_PATH//\//-}"           # /a/b/c  →  -a-b-c
PROJECT_SUFFIX="${PROJECT_SUFFIX# }"          # remove espaço inicial, se houver

echo "=== Claude Code – Extrator de Histórico ==="
echo "Repositório : $REPO_PATH"
echo "Sufixo      : $PROJECT_SUFFIX"
echo "Projetos    : $CLAUDE_PROJECTS"
echo ""

# ---------------------------------------------------------------------------
# Encontrar o diretório do projeto
# ---------------------------------------------------------------------------
PROJECT_DIR=""
if [[ -d "$CLAUDE_PROJECTS" ]]; then
  # Busca exata primeiro
  CANDIDATE="$CLAUDE_PROJECTS/$PROJECT_SUFFIX"
  if [[ -d "$CANDIDATE" ]]; then
    PROJECT_DIR="$CANDIDATE"
  else
    # Busca com glob (cobre variações de encoding)
    while IFS= read -r -d '' d; do
      BASENAME="$(basename "$d")"
      if [[ "$BASENAME" == "$PROJECT_SUFFIX" ]]; then
        PROJECT_DIR="$d"
        break
      fi
    done < <(find "$CLAUDE_PROJECTS" -mindepth 1 -maxdepth 1 -type d -print0 2>/dev/null)
  fi
fi

if [[ -z "$PROJECT_DIR" ]]; then
  echo "❌  Nenhum diretório de projeto encontrado para: $REPO_PATH"
  echo ""
  echo "Diretórios existentes em $CLAUDE_PROJECTS:"
  ls -1 "$CLAUDE_PROJECTS" 2>/dev/null || echo "  (nenhum)"
  echo ""
  echo "Dica: verifique se o caminho do repositório está correto e se você"
  echo "      já iniciou o Claude Code nesse diretório."
  exit 1
fi

echo "✅  Projeto encontrado: $PROJECT_DIR"
echo ""

# ---------------------------------------------------------------------------
# Contar sessões
# ---------------------------------------------------------------------------
JSONL_FILES=()
while IFS= read -r -d '' f; do
  JSONL_FILES+=("$f")
done < <(find "$PROJECT_DIR" -name "*.jsonl" -type f -print0 2>/dev/null | sort -z)

TOTAL="${#JSONL_FILES[@]}"
if [[ "$TOTAL" -eq 0 ]]; then
  echo "Nenhum arquivo .jsonl encontrado em $PROJECT_DIR"
  exit 0
fi

echo "Sessões encontradas: $TOTAL"
echo ""

# ---------------------------------------------------------------------------
# Criar estrutura de saída
# ---------------------------------------------------------------------------
RAW_DIR="$OUTPUT_DIR/raw_jsonl"
TXT_DIR="$OUTPUT_DIR/transcricoes"
mkdir -p "$RAW_DIR" "$TXT_DIR"

# ---------------------------------------------------------------------------
# Processar cada sessão
# ---------------------------------------------------------------------------
INDEX_FILE="$OUTPUT_DIR/indice.md"
{
  echo "# Índice de Conversas – Claude Code"
  echo ""
  echo "Repositório: \`$REPO_PATH\`"
  echo "Exportado em: $(date '+%Y-%m-%d %H:%M:%S')"
  echo "Total de sessões: $TOTAL"
  echo ""
  echo "| # | Sessão | Última modificação | Mensagens | Arquivo |"
  echo "|---|--------|-------------------|-----------|---------|"
} > "$INDEX_FILE"

SESSION_NUM=0

for JSONL in "${JSONL_FILES[@]}"; do
  SESSION_NUM=$((SESSION_NUM + 1))
  SESSION_ID="$(basename "$JSONL" .jsonl)"
  MODIFIED="$(date -r "$JSONL" '+%Y-%m-%d %H:%M:%S' 2>/dev/null || stat -c '%y' "$JSONL" 2>/dev/null | cut -d'.' -f1)"

  printf "[%d/%d] %s\n" "$SESSION_NUM" "$TOTAL" "$SESSION_ID"

  # --- 1. Copiar arquivo bruto -------------------------------------------------
  RAW_DEST="$RAW_DIR/${SESSION_ID}.jsonl"
  cp "$JSONL" "$RAW_DEST"

  # --- 2. Gerar transcrição legível -------------------------------------------
  TXT_FILE="$TXT_DIR/sessao_$(printf '%03d' "$SESSION_NUM")_${SESSION_ID}.txt"

  {
    echo "================================================================="
    echo " SESSÃO: $SESSION_ID"
    echo " Repositório: $REPO_PATH"
    echo " Última modificação: $MODIFIED"
    echo "================================================================="
    echo ""

    MSG_COUNT=0

    while IFS= read -r line; do
      [[ -z "$line" ]] && continue

      # Extrai campos com Python (disponível em qualquer Mac/Linux)
      TYPE="$(echo "$line" | python3 -c "
import sys, json
try:
    d = json.loads(sys.stdin.read())
    print(d.get('type',''))
except: print('')
" 2>/dev/null)"

      case "$TYPE" in

        # --- Mensagem do usuário ---
        user)
          MSG_COUNT=$((MSG_COUNT + 1))
          TS="$(echo "$line" | python3 -c "
import sys, json
try:
    d = json.loads(sys.stdin.read())
    print(d.get('timestamp',''))
except: print('')
" 2>/dev/null)"
          TEXT="$(echo "$line" | python3 -c "
import sys, json
try:
    d = json.loads(sys.stdin.read())
    msg = d.get('message', {})
    content = msg.get('content', '')
    if isinstance(content, list):
        parts = []
        for c in content:
            if isinstance(c, dict):
                if c.get('type') == 'text':
                    parts.append(c.get('text',''))
                elif c.get('type') == 'tool_result':
                    for item in c.get('content', []):
                        if isinstance(item, dict) and item.get('type') == 'text':
                            parts.append('[Resultado de ferramenta] ' + item.get('text','')[:500])
            elif isinstance(c, str):
                parts.append(c)
        print('\n'.join(parts))
    else:
        print(str(content))
except Exception as e:
    print('')
" 2>/dev/null)"
          echo "-----------------------------------------------------------------"
          echo "👤 USUÁRIO  [$TS]"
          echo "-----------------------------------------------------------------"
          echo "$TEXT"
          echo ""
          ;;

        # --- Mensagem do assistente ---
        assistant)
          MSG_COUNT=$((MSG_COUNT + 1))
          TS="$(echo "$line" | python3 -c "
import sys, json
try:
    d = json.loads(sys.stdin.read())
    print(d.get('timestamp',''))
except: print('')
" 2>/dev/null)"
          echo "-----------------------------------------------------------------"
          echo "🤖 CLAUDE   [$TS]"
          echo "-----------------------------------------------------------------"

          python3 - "$line" <<'PYEOF'
import sys, json
raw = sys.argv[1]
try:
    d = json.loads(raw)
    msg = d.get('message', {})
    content = msg.get('content', [])
    if not isinstance(content, list):
        content = [{'type': 'text', 'text': str(content)}]
    for block in content:
        if not isinstance(block, dict):
            print(str(block))
            continue
        btype = block.get('type', '')
        if btype == 'text':
            print(block.get('text', ''))
        elif btype == 'tool_use':
            name = block.get('name', 'desconhecida')
            inp = block.get('input', {})
            print(f"[🔧 Ferramenta: {name}]")
            if isinstance(inp, dict):
                for k, v in inp.items():
                    val = str(v)
                    if len(val) > 300:
                        val = val[:300] + '...'
                    print(f"  {k}: {val}")
            print()
except Exception as e:
    pass
PYEOF
          echo ""
          ;;

        # --- Sumário/compactação ---
        summary)
          SUMMARY="$(echo "$line" | python3 -c "
import sys, json
try:
    d = json.loads(sys.stdin.read())
    print(d.get('summary', d.get('content', '')))
except: print('')
" 2>/dev/null)"
          echo "-----------------------------------------------------------------"
          echo "📋 RESUMO DE COMPACTAÇÃO"
          echo "-----------------------------------------------------------------"
          echo "$SUMMARY"
          echo ""
          ;;

      esac

    done < "$JSONL"

    echo "================================================================="
    echo " Total de eventos de mensagem processados: $MSG_COUNT"
    echo "================================================================="

  } > "$TXT_FILE"

  # Contar mensagens reais para o índice
  REAL_MSG="$(grep -c '"type":"user"\|"type":"assistant"' "$JSONL" 2>/dev/null || echo 0)"
  TXT_BASENAME="$(basename "$TXT_FILE")"

  echo "| $SESSION_NUM | \`$SESSION_ID\` | $MODIFIED | $REAL_MSG | [📄]($TXT_BASENAME) |" >> "$INDEX_FILE"

done

# ---------------------------------------------------------------------------
# Resumo final
# ---------------------------------------------------------------------------
echo ""
echo "================================================================="
echo "✅  Exportação concluída!"
echo "================================================================="
echo ""
echo "📁 Diretório de saída : $OUTPUT_DIR"
echo "📂 JSONLs brutos      : $RAW_DIR  ($TOTAL arquivos)"
echo "📂 Transcrições txt   : $TXT_DIR  ($TOTAL arquivos)"
echo "📋 Índice             : $INDEX_FILE"
echo ""
echo "Para visualizar uma transcrição:"
echo "  less \"$TXT_DIR/sessao_001_*.txt\""
echo ""
echo "Para buscar em todas as transcrições:"
echo "  grep -r \"sua busca\" \"$TXT_DIR/\""
echo ""