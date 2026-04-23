# Integration Guide - C++ IDE for Flipper Zero

This guide explains how to integrate the C++ IDE app into your Flipper Zero development environment.

## Prerequisites

1. **Flipper Zero Firmware Repository**
   - Clone: `https://github.com/flipperdevices/flipperzero-firmware.git`
   - Follow setup instructions: https://docs.flipperzero.io/basics/compiling

2. **Build Tools**
   - ARM GCC toolchain
   - CMake 3.22 or higher
   - Make
   - Python 3.8+

3. **Environment**
   - Clone the firmware repository to your system
   - Set up environment variables as per Flipper documentation

## Integration Steps

### Option 1: Copy as External App (Recommended)

```bash
# 1. Clone the C++ IDE repository
git clone https://github.com/yourusername/FlipperApp.git

# 2. Copy to firmware directory
cp -r FlipperApp /path/to/flipperzero-firmware/applications/external/cpp_ide

# 3. Build the app
cd /path/to/flipperzero-firmware
./fbt fap_cpp_ide

# 4. Flash to device
./fbt flash_fap_cpp_ide
```

### Option 2: Build as Standalone App

```bash
# 1. Build the FAP (Flipper App Package)
cd FlipperApp
cmake -B build
cd build
make

# 2. Copy the resulting .fap file to Flipper
# Place in /apps/Tools/ directory on SD card
```

### Option 3: Integrate into Firmware

```bash
# 1. Copy to internal apps directory
cp -r FlipperApp /path/to/firmware/applications/main/cpp_ide

# 2. Add to application manifest (applications.c)
# Include the app entry point and descriptor

# 3. Rebuild firmware
cd /path/to/firmware
./fbt firmware
```

## File Structure Integration

After copying to the firmware, the directory structure should be:

```
flipperzero-firmware/
├── applications/
│   └── external/
│       └── cpp_ide/
│           ├── application.c
│           ├── application.h
│           ├── manifest.c
│           ├── CMakeLists.txt
│           ├── application.mk
│           ├── interpreter/
│           │   ├── lexer.c/h
│           │   ├── parser.c/h
│           │   ├── executor.c/h
│           │   └── value.c/h
│           ├── editor/
│           │   ├── editor.c/h
│           │   └── snippets.c/h
│           └── README.md
```

## CMakeLists Configuration

Ensure your CMakeLists.txt is compatible with the Flipper firmware build system:

```cmake
cmake_minimum_required(VERSION 3.22)

project(cpp_ide_app C)

# The firmware will handle most of the include paths
# Just ensure source files are properly listed

add_library(cpp_ide_app OBJECT
    application.c
    interpreter/value.c
    interpreter/lexer.c
    interpreter/parser.c
    interpreter/executor.c
    editor/editor.c
    editor/snippets.c
)

target_include_directories(cpp_ide_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
```

## Build Commands

### From Firmware Root

```bash
# List all available apps
./fbt list_fap

# Build the C++ IDE app
./fbt fap_cpp_ide

# Build and flash
./fbt flash_fap_cpp_ide

# Verbose build for debugging
./fbt DEBUG=1 fap_cpp_ide

# Clean build
./fbt clean
./fbt fap_cpp_ide
```

### Standalone Build (if not using firmware)

```bash
cd FlipperApp
mkdir -p build
cd build
cmake ..
make -j4
```

## Verification

### After Building

1. Check that no compilation errors occurred
2. Verify `.fap` file was created in build directory
3. File size should be reasonable (< 500KB for C++ IDE)

### After Flashing

1. Reboot Flipper Zero
2. Navigate to: **Applications** → **Tools** → **C++ IDE**
3. App should launch without errors
4. Test basic functionality:
   - Editor opens
   - Can type characters
   - Snippets menu appears
   - Can run basic code

## Troubleshooting

### Build Errors

**Error: "application.h: No such file or directory"**
- Verify file paths in CMakeLists.txt are relative and correct
- Check that all header files are in the correct directories

**Error: "undefined reference to furi_xxx"**
- Ensure Flipper SDK headers are in the include path
- This happens automatically when building through `./fbt`

**Error: "malloc/free not found"**
- Include `<stdlib.h>` at the top of files using memory functions
- Already included in provided files

### Runtime Errors

**App crashes on launch**
- Check the app allocation functions in `application.c`
- Verify Flipper GUI record operations
- Enable debug logging: `./fbt DEBUG=1 fap_cpp_ide`

**"record is not set" errors**
- Ensure records are opened in correct order
- Check that `furi_record_open` calls match with records that exist in firmware

**App doesn't respond to input**
- Verify input callback is properly registered in `view_port_input_callback_set`
- Check input queue handling

### Compilation Warnings

Some warnings are acceptable:
- Unused variables (marked with `UNUSED()`)
- Deprecation warnings (often Flipper SDK related)

Errors should always be fixed before deployment.

## Testing the App

### Unit Testing

Create a test file to verify interpreter functionality:

```c
// test_interpreter.c
#include "interpreter/executor.h"
#include "interpreter/parser.h"

int main() {
    const char* code = "int x = 5;\ncout << x << endl;";
    
    Parser parser = parser_create(code);
    ASTNode* ast = parser_parse(&parser);
    
    Executor* executor = executor_create();
    executor_execute(executor, ast);
    
    printf("%s", executor_get_output(executor));
    
    ast_node_free(ast);
    executor_free(executor);
    return 0;
}
```

Compile with: `gcc test_interpreter.c interpreter/*.c -o test_interpreter`

### On-Device Testing

1. **Simple Arithmetic**
   ```cpp
   int a = 10;
   int b = 20;
   cout << a + b << endl;
   ```

2. **Loop**
   ```cpp
   for (int i = 0; i < 5; i++) {
       cout << i << endl;
   }
   ```

3. **Conditional**
   ```cpp
   int x = 15;
   if (x > 10) {
       cout << "Greater" << endl;
   }
   ```

## Performance Optimization

### For Slower Devices

- Reduce snippet count if needed
- Minimize output buffer size
- Use smaller line count limit

### Memory Usage

Current estimates:
- Editor: ~50KB (adjustable)
- Parser/Lexer: ~30KB
- Executor: ~40KB
- Total: ~120KB (plus overhead)

Flipper Zero typically has 256KB free RAM for apps.

## Distribution

### Creating FAP Package

The Flipper build system automatically creates `.fap` files:

1. Built to: `build/f7_build/fap_cpp_ide.fap`
2. Can be distributed directly to users
3. Users place in `/apps/Tools/` on their Flipper SD card
4. App will run on any Flipper Zero with compatible firmware

### Version Management

- Update version in `manifest.c`
- Update `FAP_VERSION` in application.mk
- Document changes in CHANGELOG

## Known Issues & Limitations

1. **Nested Functions**: Not supported in this implementation
2. **String Operations**: Limited to concatenation with cout
3. **File I/O**: Not implemented (security reasons)
4. **Recursion Depth**: Limited by stack to ~32 calls
5. **Large Numbers**: Limited to int32/float precision

## Future Development

Possible enhancements:
- [ ] Full file browser integration
- [ ] Syntax highlighting
- [ ] Debugger with breakpoints
- [ ] More standard library functions
- [ ] Optimization passes
- [ ] REPL mode

## Support & Documentation

- **Main README**: Feature overview and usage
- **QUICKSTART**: Getting started guide
- **This file**: Integration instructions
- **Code comments**: Implementation details

## Related Resources

- [Flipper Zero Firmware](https://github.com/flipperdevices/flipperzero-firmware)
- [Official Documentation](https://docs.flipperzero.io/)
- [Flipper Dev Community](https://forum.flipperzero.io/)

---

**Last Updated**: 2024
**Firmware Compatibility**: Flipper Zero 0.84.0+
