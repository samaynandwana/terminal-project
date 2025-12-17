# Unix Shell Implementation

A **production-style Unix shell** implemented from scratch in C++, combining **compiler-grade parsing (Lex/YACC)** with a **low-level POSIX execution engine**. This project mirrors how real shells like `bash` translate complex command syntax into coordinated processes, pipelines, and I/O streams.

---

## High-Level Overview

The shell is a complete execution tool. User input is:

1. **Lexed and parsed** using formal grammar techniques  
2. **Lowered into structured command objects**  
3. **Transformed through multiple expansion phases**  
4. **Executed via fork–exec with precise I/O control**

The result is a shell that behaves correctly even under complex combinations of pipelines, redirection, subshells, and environment expansion.

---

## Core Capabilities 

### **Advanced Command Parsing**
- Lex/YACC-based parsing identical in spirit to real compiler frontends
- Correct handling of quotes, escapes, operators, and nested expressions
- Grammar-driven validation and structured command construction

---

### **True Unix Pipeline Execution**
- Supports **arbitrarily long pipelines** (`a | b | c | d`)
- Uses anonymous pipes to stream data between processes
- Enforces correct read/write descriptor ownership per process
- Preserves execution order and stream isolation

---

### **Full I/O Redirection Semantics**
- Input redirection (`<`)
- Output redirection with overwrite and append (`>`, `>>`)
- Error redirection (`>&`, `>>&`)
- Simultaneous redirection of stdout and stderr
- Descriptor rewiring implemented entirely with `dup2`

---

### **Foreground & Background Process Control**
- Background execution via `&`
- Foreground blocking with accurate `waitpid` handling
- Exit code propagation for scripting and variable expansion
- Background PID tracking for shell state awareness

---

### **Built-In Command Execution**
- Built-ins run **inside the shell process**, not via fork
- Ensures persistent state updates (e.g. directory changes)
- Includes directory navigation and environment manipulation
- Behavior matches real shell semantics

---

### **Multi-Stage Argument Expansion Pipeline**
Arguments undergo ordered transformation:

1. **Tilde Expansion**  
   - `~`, `~/path`, `~user/path`
2. **Environment Variable Expansion**  
   - `${VAR}`, `${?}`, `${$}`, `${_}`, and positional shell variables
3. **Subshell Evaluation**  
   - `$(command)` and backtick execution
   - Output captured and reinjected as arguments
4. **Wildcard (Glob) Expansion**  
   - `*`, `?` patterns expanded via recursive filesystem traversal

This ordering is essential to correctly model real shell behavior.

---

### **Recursive Wildcard Engine**
- Regex-based pattern matching
- Directory-by-directory descent
- Hidden file handling consistent with Unix rules
- Dynamically sized result sets
- Sorted output for deterministic execution

---

### **Interactive Shell Features**
- Raw terminal mode for character-level input
- Command history with navigation
- Line editing and cursor control
- Robust signal handling (`Ctrl+C`, `Ctrl+D`)

---

## Execution Engine Design

- File descriptors are **snapshotted and restored** around each command
- Pipes, redirections, and errors are composed without leaking state
- Child processes inherit only the descriptors they need
- Failures do not corrupt shell state


---

## Systems & Compiler Concepts

- POSIX process model (`fork`, `exec`, `wait`, `pipe`)
- File descriptor virtualization
- Signal-safe interactive programs
- Recursive filesystem algorithms
- Compiler frontends (lexing, parsing, grammar design)
- Runtime lowering from AST to OS execution
- Memory management efficiency

---
