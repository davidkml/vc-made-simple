# Commands

## init
**Usage**: `vms init`

**Description**: Creates an empty Vms repository in the current directory.
- creates `.vms`, `.vms/objects`, `.vms/branches`, and `.vms/cache` subdirectories
- initializes `.vms/index`, `.vms/log`, `.vms/HEAD`, and `.vms/branches/master` files
- initializes and saves initial commit
- prints `Repository initialized at <cwd>` upon success

**Failure cases**: 
- If there is already a repository initialized in the current directory, abort and print to standard error:
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
- If repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## stage
**Usage**: `vms stage [<filenames>] [<dirnames>]`

**Description**: Add snapshots of the contents of the given files to the staging area. 
- preprocessing and normalization of given file and directory paths is performed before staging
- if multiple arguments are given, each is staged sequentially
- if directory is given, then stage all of the files in that directory
- if a tracked file has been deleted and the deletion was staged, stage the file with a special value that tells the system to remove it from tracking
- cache the contents of the file being staged to create a snapshot and reduce the requirements during the commit operation

**Failure cases**: 
- If repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```
- if not enough arguments given, abort and print to standard error:
 ```
 Must provide filenames or directories
 usage: vms stage [<filenames>] [<dirnames>]
 ```
- if provided file is not valid (is not a file being tracked by the current commit, does not exist in current working directory, or is not a file in the staging area), then print to standard error `
<file> is not a valid or currently tracked file or directory ` and proceed to the next provided argument if any


## unstage
**Usage**: `vms unstage [<filenames>] [<dirnames>]`

**Description**: Remove the given files from the staging area
- preprocessing and normalization of given file and directory paths is performed before unstaging
- if multiple arguments are given, each is unstaged sequentially
- if a file or directory path is given that doesn't correspond to a file in the staging area, fail silently and proceed to the next argument if any

**Failure cases**: 
- If repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```
- if not enough arguments given, abort and print to standard error:
 ```
 Must provide filenames or directories
 usage: vms unstage [<filenames>] [<dirnames>]
 ```


## commit
**Usage**: `vms commit <message>`

**Description**: Save the snapshots of the files in the staging area into the repository with an associated message and create a new commit in the history
- if a file in the staging area has been staged with the special value corresponding to deletion, remove the file from tracking
- update the internal map of the commit with the contents of the staging area
- move the snapshots from the `vms/cache` to the more permanent `vms/objects` directory
- write to the `vms/objects` directory using the first two characters of of the hash as a tag
- change permissions of snapshots after moving to remove write access
- clear cache
- clear staging area
- update the log
- update the position of the branch pointed to by HEAD
- save the commit with an updated internal map into the  `vms/objects` directory

**Failure cases**: 
- If repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```
- If not enough arguments are given, abort and print to standard error:
```
Must provide a message with your commit
usage: vms commit <message>
```
- if there are no changes staged to commit, abort and print to standard error:
```
No changes staged to commit
```
## log
**Usage**: `vms log`

**Description**: 

**Failure cases**: 
- If repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## checkout
**Usage**: `vms checkout files <commitid> [<filenames>]` **or** `vms checkout branch <branchname>`

**Description**:

**Failure cases**: 
- If repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## mkbranch
**Usage**: `vms mkbranch <branchname> [commitid]`

**Description**:

**Failure cases**: 
- If repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## rmbranch
**Usage**: `vms rmbranch <branchname>`

**Description**:

**Failure cases**: 
- If repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## info
**Usage**: `vms info <commitid> [file]`

**Description**:

**Failure cases**: 
- If repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## merge
**Usage**: `vms merge <branchname>`

**Description**:

**Failure cases**: 
- If repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```
