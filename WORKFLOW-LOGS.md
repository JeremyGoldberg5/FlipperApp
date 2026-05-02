# Automated Workflow Log Fetching

This repo includes automated scripts to fetch and analyze GitHub Actions workflow logs without manual copy-pasting.

## Setup (One-time)

### 1. Install GitHub CLI (`gh`)

**Windows:**
```powershell
winget install GitHub.cli
# OR if you use Chocolatey:
choco install gh
```

**macOS:**
```bash
brew install gh
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get install gh
```

### 2. Authenticate

```bash
gh auth login
```

Follow the prompts. You can use your existing GitHub credentials.

### 3. Verify Installation

```bash
gh auth status
```

You should see: `Logged in to github.com as <your-username>`

---

## Usage

### PowerShell (Windows - Recommended)
```powershell
# Show last 100 lines of latest run
.\fetch-logs.ps1

# Analyze for specific errors
.\fetch-logs.ps1 -Analyze

# Show all output
.\fetch-logs.ps1 -ShowAll
```

### Bash (Linux/macOS/WSL)
```bash
# Show last 100 lines of latest run
./fetch-logs.sh

# Analyze for specific errors
./fetch-logs.sh --analyze

# Show all output
./fetch-logs.sh --show-all
```

---

## What It Does

1. **Fetches the latest workflow run** from GitHub Actions
2. **Downloads full logs** (~1-2 seconds)
3. **Shows tail (last 100 lines)** by default
4. **Saves to `.logs/` directory** for reference
5. **Analyzes for common errors** when you use `-Analyze`

### Analysis Mode

The `-Analyze` flag checks for:
- ✅ cpp_ide.fap successfully built
- ❌ cpp_ide.fap generation failure
- ✅ cpp_ide copied to applications/examples/
- ✅ application.fam file exists
- ❌ Build errors and failures

Example:
```powershell
.\fetch-logs.ps1 -Analyze

Latest run: Build C++ IDE FAP (ID: 12345678)
Status: completed
Time: 2026-05-02T15:30:00Z

✓ Logs saved to: .logs/run-12345678.txt

=== ERROR ANALYSIS ===
❌ cpp_ide.fap was not generated
✅ cpp_ide directory exists
✅ application.fam exists

Checking for build failures...
(error messages shown here)
```

---

## Workflow

1. Push code to GitHub: `git push origin main`
2. GitHub Actions automatically runs
3. Once complete (~5 minutes), run the fetch script:
   ```powershell
   .\fetch-logs.ps1 -Analyze
   ```
4. See results instantly
5. Get full logs with: `Get-Content .logs/run-<ID>.txt`

---

## Logs Directory

All logs are saved to `.logs/` with the format: `run-<WORKFLOW_ID>.txt`

To view all previous logs:
```bash
ls -la .logs/
```

To search across all logs:
```bash
grep -r "error" .logs/
```

---

## Troubleshooting

### "gh: command not found"
Install GitHub CLI (see Setup section above)

### "ERROR: gh not authenticated"
Run: `gh auth login`

### "ERROR: Repository not found"
Ensure you're in the FlipperApp directory and `gh` is authenticated to the correct account

### "No logs downloaded"
Ensure the workflow has completed. Check:
```powershell
gh run list --repo JeremyGoldberg5/FlipperApp --limit 5
```

---

## Next Steps

Once you have `gh` installed, you'll never need to:
- Copy/paste build output manually
- Worry about missing log files
- Wait for me to ask for logs

Just run the script after each push and we'll have instant, complete visibility into what's happening!
