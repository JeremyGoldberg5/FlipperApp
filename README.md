# C++ IDE for Flipper Zero

A lightweight C++ IDE that runs on Flipper Zero, allowing users to write, compile, and execute simple C++ programs directly on the device.

## Features

- **Code Editor**: Full-featured text editor with line numbers and keyboard navigation
- **Code Snippets**: Pre-defined code templates for common C++ patterns
- **Interpreter**: Built-in C++ interpreter for executing code
- **Output Display**: View program output in real-time

## Architecture

### Core Components

1. **Interpreter**
   - `lexer.c/h` - Tokenizes C++ code
   - `parser.c/h` - Parses tokens into an Abstract Syntax Tree (AST)
   - `executor.c/h` - Executes the AST
   - `value.c/h` - Runtime value system (int, float, string, array)

2. **Editor**
   - `editor.c/h` - Text editing operations and line management
   - `snippets.c/h` - Pre-defined code snippets

3. **Application**
   - `application.c/h` - Main app logic and UI integration
   - `manifest.c` - Flipper Zero app descriptor

## Supported C++ Features

### Data Types
- `int` - 32-bit integers
- `float` - Floating-point numbers
- `string` - Text strings
- `vector` - Dynamic arrays (basic support)

### Control Flow
- `if` / `else` - Conditional execution
- `for` - Loop with initialization, condition, increment
- `while` - Loop with condition
- `break` / `continue` - Loop control (with extensions)

### Functions
- User-defined functions with parameters and return values
- Built-in functions: `sqrt()`, `sin()`, `cos()`, `abs()`

### I/O
- `cout << expression` - Output text and values
- `cin >> variable` - Input values (basic support)

### Operators
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparison: `<`, `<=`, `>`, `>=`, `==`, `!=`
- Logical: `&&`, `||`, `!`

## Usage

### Creating New Programs

1. Select "New Code" from the main menu
2. Use the editor to write C++ code:
   - **UP/DOWN** arrows - Move between lines
   - **LEFT/RIGHT** arrows - Move cursor within a line
   - **BACK** - Delete character before cursor
   - **OK** - Access menu

### Using Code Snippets

1. Press OK in the editor to access the menu
2. Select "Code Snippets"
3. Navigate with **UP/DOWN** and press **OK** to insert

Available snippets:
- If Statement
- If-Else
- For Loop
- While Loop
- Function
- Variable declarations
- cout / cin usage
- Math examples
- Common algorithms (Sum, Factorial, etc.)

### Running Programs

1. Write your code in the editor
2. Press OK and select "Run Code"
3. View output on the output screen
4. Press OK or BACK to return to editor

### Saving/Loading Files

- Programs are saved to `/ext/cpp_ide/` directory on the Flipper SD card
- Use "Save File" to persist your work
- Use "Load File" to open previously saved programs

## Code Examples

### Hello World
```cpp
using namespace std;

cout << "Hello, World!" << endl;
```

### Sum of Numbers
```cpp
using namespace std;

int sum = 0;
for (int i = 1; i <= 100; i++) {
    sum = sum + i;
}
cout << sum << endl;
```

### Factorial
```cpp
using namespace std;

int n = 5;
int fact = 1;
for (int i = 1; i <= n; i++) {
    fact = fact * i;
}
cout << fact << endl;
```

## Limitations

- **Simplified C++**: Not full C++20 standard - supports procedural code without classes or templates
- **Memory Constraints**: Limited by Flipper Zero's 256KB RAM
- **Runtime**: Programs execute within the interpreter, not natively compiled
- **Libraries**: Only basic math and I/O functions available
- **Size Limits**: Maximum 8KB per source file

## Building

### Requirements
- Flipper Zero development environment
- ARM GCC toolchain
- CMake 3.22+

### Build Steps
```bash
# Clone the Flipper firmware
git clone https://github.com/flipperdevices/flipperzero-firmware.git

# Copy this app to the firmware
cp -r FlipperApp firmware/applications/external/cpp_ide

# Build the app
cd firmware
./fbt fap_cpp_ide
```

## Future Enhancements

- [ ] File browser for managing programs
- [ ] Syntax highlighting
- [ ] Debugger with breakpoints
- [ ] More built-in functions (pow, log, etc.)
- [ ] Array/vector operations
- [ ] Input validation and error messages
- [ ] Standard library functions
- [ ] Performance optimizations

## License

MIT License - See LICENSE file for details

## Author

Jeremy Goldberg
