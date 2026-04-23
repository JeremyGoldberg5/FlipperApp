#include "snippets.h"

const Snippet snippets[] = {
    {
        .name = "If Statement",
        .code = "if (condition) {\n    // code\n}",
    },
    {
        .name = "If-Else",
        .code = "if (condition) {\n    // code\n} else {\n    // code\n}",
    },
    {
        .name = "For Loop",
        .code = "for (int i = 0; i < 10; i++) {\n    // code\n}",
    },
    {
        .name = "While Loop",
        .code = "while (condition) {\n    // code\n}",
    },
    {
        .name = "Function",
        .code = "void myFunction() {\n    // code\n}",
    },
    {
        .name = "Function Call",
        .code = "myFunction();",
    },
    {
        .name = "Variable (int)",
        .code = "int x = 0;",
    },
    {
        .name = "Variable (float)",
        .code = "float x = 0.0;",
    },
    {
        .name = "Variable (string)",
        .code = "string name = \"text\";",
    },
    {
        .name = "Output (cout)",
        .code = "cout << \"text\" << endl;",
    },
    {
        .name = "Math Function",
        .code = "int result = sqrt(16);",
    },
    {
        .name = "Hello World",
        .code = "cout << \"Hello, World!\" << endl;",
    },
    {
        .name = "Sum Loop",
        .code = "int sum = 0;\nfor (int i = 1; i <= 100; i++) {\n    sum = sum + i;\n}\ncout << sum << endl;",
    },
    {
        .name = "Factorial",
        .code = "int n = 5;\nint fact = 1;\nfor (int i = 1; i <= n; i++) {\n    fact = fact * i;\n}\ncout << fact << endl;",
    },
};

const size_t snippets_count = sizeof(snippets) / sizeof(snippets[0]);
