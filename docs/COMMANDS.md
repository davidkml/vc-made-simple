# Commands

## init
**Usage**: `vms init`

**Description**: Creates an empty Vms repository in the current directory.
- creates `.vms`, `.vms/objects`, `.vms/branches`, and `.vms/cache` subdirectories
- initializes `.vms/index`, `.vms/log`, `.vms/HEAD`, and `.vms/branches/master` files
- initializes and saves initial commit
- prints `Repository initialized at <cwd>` upon success

**Failure cases**: 
- if there is already a repository initialized in the current directory, abort and print to standard error:
```
Repository is already initialized
  (type "vms" in command prompt to display a summary of available commands)
```
## status
**Usage**: `vms status`

**Description**: Displays the status (current branch, other branches, staged files, unstaged changes, untracked files, subdirectories) of the working tree with format:
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
- if repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## stage
**Usage**: `vms stage [<filenames>] [<dirnames>]`

**Description**: Adds snapshots of the contents of the given files to the staging area. 
- preprocessing and normalization of given file and directory paths is performed before staging
- if multiple arguments are given, each is staged sequentially
- if directory is given, then stage all of the files in that directory
- if a tracked file has been deleted and the deletion was staged, stage the file with a special value that tells the system to remove it from tracking
- cache the contents of the file being staged to create a snapshot and reduce the requirements during the commit operation

**Failure cases**: 
- if repository is not in initialized, abort and print to standard error:
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

**Description**: Removes the given files from the staging area
- preprocessing and normalization of given file and directory paths is performed before unstaging
- if multiple arguments are given, each is unstaged sequentially
- if a file or directory path is given that doesn't correspond to a file in the staging area, fail silently and proceed to the next argument if any

**Failure cases**: 
- if repository is not in initialized, abort and print to standard error:
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

**Description**: Saves the snapshots of the files in the staging area into the repository with an associated message and create a new commit in the history
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
- if repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```
- if not enough arguments are given, abort and print to standard error:
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

**Description**: Displays a chronological log of the commit history with format:
```
===
commit  <commit_id>
Date    <date_and_time>
parent  <parent_commit_id>
		[parent_commit_id2]

    <message>
[...]
```
- if the commit has no second parent, then it is not displayed

**Failure cases**: 
- if repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```

## checkout files
**Usage**: `vms checkout files <commitid> [<filenames>]`

**Description**: Restores the version of all (or optionally, only the given) files as they exist in the commit corresponding to the given id, overwriting the versions in the current working directory, if they exist.
- output warning prompt to user along with information about files that may be updated
- if user answers `n`, abort without changing state
- if user answers `y`, write or overwrite files in the current directory with the versions as they exist in the commit with the given id.

**Failure cases**: 
- if repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```
- if not enough arguments are given, abort and print to standard error:
```
Must provide id of commit to checkout and optionally files
usage: vms checkout files <commitid> [<filenames>]
``` 
- if the given commit id does not uniquely match a commit in the repository, abort and print to standard error:
```
Provided commit id <commitid> generated ambiguous or no matches
Please provide more characters or verify the accuracy of your input
  (use "vms log" to see a log of commits)
```

## checkout branch
**Usage**: `vms checkout branch <branchname>`

**Description**: Switches branches and update files in the current working directory with the versions as they exist in the given branch.
- output warning prompt to user along with information about files that may be updated
- if user answers `n`, abort without changing state
- if user answers `y`, move the HEAD pointer to point to the given branch and write or overwrite files in the current directory with the versions as they exist in the commit with the given id.
- clear the staging area.
 
**Failure cases**: 
- if repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```
- if not enough arguments are given, abort and print to standard error:
```
Must provide name of branch to checkout
usage: vms checkout branch <branchname>
```
- if the given branch does not exist, abort and print to standard error:
```
There is no branch named <branchname>
  (use "vms status" to see list of available branches)
```
- if the given branch is the current branch, abort and print to standard error:
```
Already on branch <branchname>
```

## mkbranch
**Usage**: `vms mkbranch <branchname> [commitid]`

**Description**: Creates a new branch at the same location of your current branch (or optionally, at the location of the commit corresponding to the given id).

**Failure cases**: 
- if repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```
- if not enough arguments are given, abort and print to standard error:
```
Must provide name of branch to create and optionally the id to create it at
usage: vms mkbranch <branchname> [commitid]
```
- if the given branch already exists, abort and print to standard error:
```
Branch <branchname> already exists
  (use "vms checkout branch <branchname>" to move to it)
```
- if the given commit id does not uniquely match a commit in the repository, abort and print to standard error:
```
Branch not created
Provided commit id <commitid> generated ambiguous or no matches
Please provide more characters or verify the accuracy of your input
  (use "vms log" to see log of commits)
```

## rmbranch
**Usage**: `vms rmbranch <branchname>`

**Description**: Remove the branch with the given name.

**Failure cases**: 
- if repository is not in initialized, abort and print to standard error:
```
Repository is not initialized
  (use "vms init" to initialize repository)
```
- if not enough arguments are given, abort and print to standard error:
```
Must provide name of branch to remove
usage: vms rmbranch <branchname>
```
- if given branch does not exist, abort and print to standard error:
```
No branch named <branchname>
  (use "vms status" to see list of available branches)
```
- if given branch is the current branch, abort and print to standard error:
```
Currently on branch <branchname>
  (use "vms checkout branch <otherbranch>" to move to another branch)
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
