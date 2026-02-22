# Mini Shell

A Unix shell implementation written in C++ from scratch for learning purposes. Supports pipelines, I/O redirection, job control, and signal handling.

## Requirements

- Linux or macOS
- g++ with C++17 support
- make

## Build

```bash
git clone https://github.com/ValijonSharifjon/mini-shell.git
cd mini-shell
make
```

## Run

```bash
./myshell
```

To exit:

```bash
myshell>> exit
```

## Features

### Basic commands

Any command available on your system works:

```bash
myshell>> ls
myshell>> echo hello world
myshell>> cat file.txt
```

### Built-in commands

| Command | Description |
|---------|-------------|
| `cd <dir>` | Change directory |
| `pwd` | Print current directory |
| `jobs` | List background jobs |
| `fg %<n>` | Bring job to foreground |
| `bg %<n>` | Resume job in background |
| `exit` | Exit the shell |

### Pipelines

Chain commands together with `|`:

```bash
myshell>> ls | grep .cpp
myshell>> cat file.txt | sort | uniq
myshell>> ps aux | grep sleep
```

### I/O Redirection

```bash
myshell>> echo hello > output.txt       # write stdout to file
myshell>> cat < input.txt               # read stdin from file
myshell>> ls 2> errors.txt             # write stderr to file
myshell>> ls 2>&1                      # redirect stderr to stdout
myshell>> ls | grep txt > result.txt   # pipe and redirect together
```

### Background processes

Run a command in the background with `&`:

```bash
myshell>> sleep 100 &
[1] 12345
myshell>>                  # prompt returns immediately
```

### Job control

```bash
myshell>> sleep 100 &
[1] 12345
myshell>> sleep 200 &
[2] 12346
myshell>> jobs
[1]  Running    sleep 100
[2]  Running    sleep 200
myshell>> fg %1            # bring sleep 100 to foreground
myshell>> bg %1            # resume in background after Ctrl+Z
```

### Signals

| Shortcut | Action |
|----------|--------|
| `Ctrl+C` | Kill foreground process (shell stays alive) |
| `Ctrl+Z` | Suspend foreground process |
| `Ctrl+\` | Ignored |

## Clean build

```bash
make clean
make
```
