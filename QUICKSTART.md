# Quick Start Guide - C++ IDE for Flipper Zero

## Installation

1. **Set up Flipper Zero Development Environment**
   ```bash
   git clone https://github.com/flipperdevices/flipperzero-firmware.git
   cd flipperzero-firmware
   ./fbt --version
   ```

2. **Copy the C++ IDE App**
   ```bash
   cp -r /path/to/FlipperApp applications/external/cpp_ide
   ```

3. **Build the App**
   ```bash
   ./fbt fap_cpp_ide
   ```

4. **Upload to Flipper Zero**
   ```bash
   ./fbt flash_fap_cpp_ide
   ```

## First Program: Hello World

1. Launch the C++ IDE app from your Flipper Zero
2. Select "New Code"
3. Type the following:
   ```cpp
   using namespace std;
   cout << "Hello, World!" << endl;
   ```
4. Press **OK** → **Run Code**
5. You should see "Hello, World!" in the output

## Editor Controls

| Button | Action |
|--------|--------|
| **UP/DOWN** | Move cursor between lines |
| **LEFT/RIGHT** | Move cursor within line |
| **BACK** | Delete character before cursor |
| **OK** | Open menu (Run, Snippets, Save, Load) |
| **BACK (long)** | Return to main menu |

## Menu Options

- **1. New Code** - Start with empty editor
- **2. Load File** - Open a saved program
- **3. Save File** - Save current program
- **4. Run Code** - Execute the code
- **5. Insert Snippet** - Browse and insert code templates

## Code Snippet Examples

All snippets are pre-loaded and available for quick insertion:

### Basic Loop
```cpp
for (int i = 0; i < 10; i++) {
    cout << i << endl;
}
```

### Sum Numbers
```cpp
int sum = 0;
for (int i = 1; i <= 100; i++) {
    sum = sum + i;
}
cout << sum << endl;
```

### Factorial Calculation
```cpp
int n = 5;
int fact = 1;
for (int i = 1; i <= n; i++) {
    fact = fact * i;
}
cout << fact << endl;
```

## Tips & Tricks

1. **Use Snippets** - Press OK to access snippets for common patterns
2. **Start Simple** - Begin with basic arithmetic and loops
3. **Variable Names** - Keep names short (space is limited)
4. **Output Often** - Use `cout` to debug what's happening
5. **Save Your Work** - Programs are stored on the SD card

## Common Errors

| Error | Cause | Solution |
|-------|-------|----------|
| "Syntax Error" | Invalid C++ syntax | Check brackets, semicolons |
| "No Output" | Program ran but produced no output | Add `cout <<` statements |
| "Memory Error" | Program too large | Break into smaller pieces |
| "Division by Zero" | Dividing by zero | Add condition checks |

## Program Storage

- Programs are saved to: `/ext/cpp_ide/` on your Flipper SD card
- Each file has `.cpp` extension
- Maximum file size: 8KB
- Maximum output: 4KB

## Performance Notes

- Simple programs (loops, arithmetic) run nearly instantly
- Complex nested loops may take a few seconds
- Output display has a 4KB limit
- Maximum 256 variables per program

## Advanced Usage

### Using Math Functions
```cpp
int root = sqrt(16);  // root = 4
float sine = sin(0);  // sine = 0.0
int absolute = abs(-5);  // absolute = 5
```

### Working with Arrays
```cpp
vector<int> numbers;
// Push values and use them
```

### Conditional Logic
```cpp
if (x > 10) {
    cout << "Greater" << endl;
} else {
    cout << "Less or Equal" << endl;
}
```

## Next Steps

- Experiment with different algorithms
- Combine loops and conditionals
- Use snippets as templates for new ideas
- Check the README.md for full feature documentation

---

For issues or questions, refer to README.md or check the code examples in the editor!
