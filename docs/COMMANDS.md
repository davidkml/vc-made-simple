# Commands

## init
**Usage**: `vms init`

**Description**: Creates an empty Vms repository in the current directory.
- creates `.vms`, `.vms/objects`, `.vms/branches`, and `.vms/cache` subdirectories
- initializes `.vms/index`, `.vms/log`, `.vms/HEAD`, and `.vms/branches/master` files
- initializes and saves initial commit
- prints `Repository initialized at <cwd>` upon success

**Failure cases**: 
- If there is already a repository initialized in the current directory, then abort and print to standard error:
```
Repository is already initialized
  (type "vms" in command prompt to display a summary of available commands)
```
## status
**Usage**: `vms status`

**Description**: Display the status (current branch, other branches, staged files, unstaged changes, untracked files, subdirectories) of the working tree with format:
```
On branch <current_branch>  [<commitid>]

Other branches
    <branch>  [<commitid>]
    [...]

Changes staged for commit
  (use "vms unstage <file>" to unstage changes to file)
  (use "vms commit <message>" to commit all staged changes)

    <mod_type>:    <file>
    [...]

Changes not yet staged for commit
  (use "vms stage <file>" to update or stage changes to be committed)

    <mod_type>:    <file>
    [...]
        
Untracked files
  (use "vms stage <file>" to include file to be committed and tracked)

    <file>
    [...]
    
Sub-directories
    <dir>
    [...]
```

- any heading with no elements under it is hidden
- detects modifications to files tracked by latest commit or in the staging area, including `modified` and `deleted`
- staged files not tracked by latest commit are labeled as `new`
- if a file has been staged (cached) but has since been deleted in the current working directory, it will show up under the` Changes not yet staged for commit` header as `deleted` and staging the file again will remove it from the staging area
- staging a file that is not tracked moves it from the `Untracked files` header into the `Changes staged for commit` header

**Failure cases**: 
- If repository is not in initialized, then abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## stage
**Usage**: `vms stage [<filenames>] [<dirnames>]`

**Description**:

**Failure cases**: 
- If repository is not in initialized, then abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## unstage
**Usage**: `vms unstage [<filenames>] [<dirnames>]`

**Description**:

**Failure cases**: 
- If repository is not in initialized, then abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## commit
**Usage**: `vms commit <message>`

**Description**:

**Failure cases**: 
- If repository is not in initialized, then abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## log
**Usage**: `vms log`

**Description**:

**Failure cases**: 
- If repository is not in initialized, then abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## checkout
**Usage**: `vms checkout files <commitid> [<filenames>]` **or** `vms checkout branch <branchname>`

**Description**:

**Failure cases**: 
- If repository is not in initialized, then abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## mkbranch
**Usage**: `vms mkbranch <branchname> [commitid]`

**Description**:

**Failure cases**: 
- If repository is not in initialized, then abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## rmbranch
**Usage**: `vms rmbranch <branchname>`

**Description**:

**Failure cases**: 
- If repository is not in initialized, then abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## info
**Usage**: `vms info <commitid> [file]`

**Description**:

**Failure cases**: 
- If repository is not in initialized, then abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## merge
**Usage**: `vms merge <branchname>`

**Description**:

**Failure cases**: 
- If repository is not in initialized, then abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```
