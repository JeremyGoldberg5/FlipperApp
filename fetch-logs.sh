#!/bin/bash
# Fetch and analyze the latest GitHub Actions workflow run logs
# Usage: ./fetch-logs.sh [--analyze] [--show-all]

set -e

REPO="JeremyGoldberg5/FlipperApp"
LOGS_DIR=".logs"
ANALYZE=false
SHOW_ALL=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --analyze) ANALYZE=true; shift ;;
        --show-all) SHOW_ALL=true; shift ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

# Check if gh is installed
if ! command -v gh &> /dev/null; then
    echo "ERROR: gh CLI not found. Install it first:"
    echo "  Windows: winget install GitHub.cli"
    echo "  macOS: brew install gh"
    echo "  Linux: sudo apt-get install gh"
    exit 1
fi

# Check if authenticated
if ! gh auth status > /dev/null 2>&1; then
    echo "ERROR: gh not authenticated. Run: gh auth login"
    exit 1
fi

# Create logs directory
mkdir -p "$LOGS_DIR"

echo "Fetching latest workflow runs for $REPO..."

# Get the latest workflow run
LATEST_RUN=$(gh run list --repo "$REPO" --limit 1 --json databaseId,name,status,createdAt --jq '.[0]')
RUN_ID=$(echo "$LATEST_RUN" | jq -r '.databaseId')
RUN_NAME=$(echo "$LATEST_RUN" | jq -r '.name')
RUN_STATUS=$(echo "$LATEST_RUN" | jq -r '.status')
RUN_TIME=$(echo "$LATEST_RUN" | jq -r '.createdAt')

echo "Latest run: $RUN_NAME (ID: $RUN_ID)"
echo "Status: $RUN_STATUS"
echo "Time: $RUN_TIME"
echo ""

# Fetch logs
echo "Downloading logs..."
LOG_FILE="$LOGS_DIR/run-${RUN_ID}.txt"
gh run logs "$RUN_ID" --repo "$REPO" > "$LOG_FILE" 2>&1

echo "✓ Logs saved to: $LOG_FILE"
echo ""

# Show summary
if [ "$SHOW_ALL" = false ]; then
    echo "=== TAIL (Last 100 lines) ==="
    tail -100 "$LOG_FILE"
    echo ""
fi

# Analyze for errors if requested
if [ "$ANALYZE" = true ]; then
    echo "=== ERROR ANALYSIS ==="

    # Look for known error patterns
    if grep -q "FATAL: cpp_ide.fap was not built" "$LOG_FILE"; then
        echo "❌ cpp_ide.fap was not generated"
        echo ""
        echo "Checking for build failures..."
        grep -i "error\|failed\|fatal" "$LOG_FILE" | grep -v "^CC\|^LINK\|^FAP\|^APPMETA" | head -20
    fi

    if grep -q "cpp_ide.fap found" "$LOG_FILE"; then
        echo "✅ cpp_ide.fap successfully built!"
        echo ""
        grep "SUCCESS:" "$LOG_FILE"
    fi

    if grep -q "✓ cpp_ide directory exists" "$LOG_FILE"; then
        echo "✅ cpp_ide was copied to applications/examples/"
    else
        echo "❌ cpp_ide directory was not found"
    fi

    if grep -q "application.fam exists" "$LOG_FILE"; then
        echo "✅ application.fam file found"
    else
        echo "❌ application.fam file not found"
    fi
fi

echo ""
echo "To view full logs: cat $LOG_FILE"
echo "To analyze errors: ./fetch-logs.sh --analyze"
echo "To see everything: ./fetch-logs.sh --show-all"
