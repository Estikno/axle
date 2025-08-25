# 🛠 Error Handling Guide for the Engine

In this engine, we use four main mechanisms to handle errors and unexpected conditions: `AX_PANIC`, `AX_ASSERT`, `AX_ENSURE` and `Expected<T>`. Each has a clear purpose and helps maintain code safety and robustness.

## 1 `AX_PANIC` (immediate crash)

**What it is:** Immediately stops the program with a panic message, similar to Rust's `panic!`. Even if you add a *try* and *catch* block, the program will still terminate.
**When to use:**

- For critical unrecoverable errors where continuing execution is unsafe.
- Usually used for situations that should **never happen** or indicate a critical bug.
- You want to define custom logic before terminating, like logging or cleaning up resources.

**Examples:**

```cpp
// The program cannot continue if the window fails to initialize
if (!window->IsInitialized()) {
    AX_PANIC("Window failed to initialize");
}
```

```cpp
// If certain shader is required and fails to compile, we must stop immediately
if (shaderCompilationFailed) {
    AX_PANIC("Shader compilation failed for {0}", shaderName);
}
```

---

## 2 `AX_ASSERT` (DEBUG only)

**What it is:** Internal contract check based on `AX_PANIC`. It behaves like the normal *assert* but the program will terminate immediately on error. Important to state that it's only active in debug mode; disappears in release.  
**When to use:**  

- To catch bugs in the engine or internal inconsistencies that **should never happen**.
- To verify internal invariants of the engine.  
- Its objective is to protect the **engine developer**, not the user.

**Examples:**

```cpp
// Some other part of the engine needs to check this id that should always be valid
AX_ASSERT(id < MAX_ENTITIES, "Entity ID out of range");
```

```cpp
// This texture should have already been initialized by other sections of the engine
AX_ASSERT(texture != nullptr, "Texture not initialized");
```

---

## 3 `AX_ENSURE`

**What it is:** Same as `AX_ASSERT` but it remains active in release mode.
**When to use:**

- For errors that may occur due to **incorrect use of the engine by the user**.
- Its objective is to protect the **engine user**, not the developer.
- It's main objective is to prevent the engine from entering an invalid state due to user mistakes.

**Examples:**

```cpp
// The user might have provided an invalid shader name or entity ID
AX_ENSURE(IsShaderCompiled(shader), "Shader {0} not compiled.", shaderName);
```

```cpp
// The user might forget to register a component before using it, which would break the engine
AX_ENSURE(IsComponentRegistered<T>(), "Component {0} is used before being registered.", typeid(T).name());
```

---

## 4 `Expected<T>` (recoverable errors)

**What it is:** A container that can hold a value or an error. Useful when an operation can **fail in a normal, expected way**. It's based on rust's result type.
**When to use:**

- Operations that may fail due to normal runtime conditions.
- When you want the user to handle the error without crashing the engine.

**Examples:**

```cpp
Expected<f32> div = Divide(30.0f, 0.0f);
if (!div.IsValid()) {
    // The user has attempted to divide by zero
    // ... blah blah ...
}
```

```cpp
Expected<Entity&> entity = GetEntityById(id);
if(!entity.IsValid()){
    // The user has incorrectly requested a non-existent entity
    // ... blah blah ...
}
```

---

## 📌 Quick Reference Table

| Error Type                         | Example                                           | Mechanism     |
| ---------------------------------- | ------------------------------------------------- | ------------- |
| Critical unrecoverable error with custom logic       | Window not initialized   | `AX_PANIC`    |
| Internal bug / invariant violation | Out-of-range IDs, null pointers inside the engine | `AX_ASSERT`   |
| Possible critical API misused by user                 | Unregistered components, wrong window creation arguments| `AX_ENSURE`   |
| Expected / recoverable condition   | Missing optional files, dividing by zero| `Expected<T>` |

---

⚡ **Convention:**

- `AX_PANIC` handles **critical errors that require immediate program termination** (More freedom to add custom logic before crashing).
- `AX_ASSERT` protects the **engine developer**.
- `AX_ENSURE` protects the **engine user**.
- `Expected<T>` handles **normal runtime errors**.
