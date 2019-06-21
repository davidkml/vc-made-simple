# Tutorial

**Contents**<br>
[Creating a repository](#creating-a-repository)<br>
[Checking the status of your files](#checking-the-status-of-your-files)<br>
[Staging and unstaging files for commit](#staging-and-unstaging-files-for-commit)<br>
[Committing your changes](#committing-your-changes)<br>
[Browsing the history of your project](#browsing-the-history-of-your-project)<br>
[Restoring previous versions of your files](#restoring-previous-versions-of-your-files)<br>
[Creating and using branches](#creating-and-using-branches)<br>
[Next steps](#next-steps)<br>

This tutorial will cover the basic commands and features you will use to manage content with Vms. 

Through the course of the tutorial, you will learn how to get started using Vms to:
- Initialize repositories
- Stage and unstage files
- Commit changes
- Browse and inspect your commit history
- "Check out" versions of your files
- Create and merge branches of development

Because Vms is a command-line tool, you must possess some basic familiarity with the command line in order to use it. However, even if you are not as comfortable with the command line, you should still be able to follow along conceptually with this tutorial.

For those interested in reading a more complete specification of its behavior and nuances, please take a look at the [command specification](COMMANDS.md). 

## Creating a repository

Before using Vms in a project, we must first initialize a repository at the root directory of our project using the command `vms init`.

As an example, suppose the project of interest lives in a directory with the path `/Users/user/my_project`.

Then, to initialize a Vms repository for our project, we would move into your project's directory with:
```bash
$ cd /Users/user/my_project
```
and type the command:
```
$ vms init
```
This creates a new subdirectory named `.vms` and performs all of the necessary initializations required for setting up your repository. 

## Checking the status of your files

We may check the status of our project files by typing the command `vms status`.

If our project has been newly created and its directory is empty, using `vms status` we will see something that looks like the following:
```
$ vms status
On branch master  [a7ef0e]

No changes to staged or tracked files, working tree clean
```

However, as files are added to the directory, they will be displayed in the output of `vms status`. For example, if our project directory contained files named `README.md` and `CONTRIBUTING.md`, as well as subdirectories named `src/` and `tests/` each containing their own set of files, they will appear in the status output like this:
```
$ vms status
On branch master  [a7ef0e]
No changes to staged or tracked files, working tree clean

Untracked files
  (use "vms stage <file>" to include file to be committed and tracked)

    CONTRIBUTING.md
    README.md

Sub-directories
    src/
    tests/
```

The `vms status` command provides a lot of useful information. Based on the output above we are given:
- the name of our current branch in addition to a truncated id of the commit at which the branch is presently located. *By default, every repository is initialized with a branch named* `master` *that initially points to a dummy commit*.
- the knowledge that the working directory is clean. This means that there have been no modifications to the files being tracked by Vms and there is nothing new to commit. 
- a list of files in our current working directory that are not being tracked by Vms, as well as a list of subdirectories.

It is important to understand that at this point, immediately following initialization, the repository is not yet tracking any files. Like Git, Vms does not begin tracking any files without our explicit command.

As we begin adding things to our repository, `vms status` will provide even more information about other branches, the staging area, and modifications to tracked files. The command also offers helpful suggestions regarding what we can do provided this information.

## Staging and unstaging files for commit

Before we can commit file changes to our repository, we must add them to the staging area.

The staging area is an intermediate area that serves as an index of all the files you want included in your next commit. When you stage a file, it creates a temporary snapshot of the file as it exists currently. This snapshot will not be saved unless we commit. Thus, the staging area can serve as setting in which we can organize, manage, and review our changes before committing them to our repository.

Files are staged using the command `vms stage`.  The command can take singular or multiple files and directories as arguments. If a directory is given as an argument to `vms stage`, then the command will stage all the files in that directory. We are not required to stage all our files at once; the state of the staging area will persist until we are ready to commit.

For example, suppose we wanted to stage `README.md`, `CONTRIBUTING.md`, and all of the files inside the `src/` subdirectory. Using `vms stage` and then `vms status`, we will receive an output that looks like: 

```
$ vms stage README.md CONTRIBUTING.md src/
$ vms status
On branch master  [a7ef0e]
Changes staged for commit
  (use "vms unstage <file>" to unstage changes to file)
  (use "vms commit <message>" to commit all staged changes)

    new file:    CONTRIBUTING.md
    new file:    README.md
    new file:    src/bar
    new file:    src/foo

Sub-directories
    src/
    tests/
```

We now see the files moved from being displayed under the `Untracked files` heading to being displayed under the `Changes staged for commit` heading. This heading denotes that they are in our staging area. We also see the type of modification the file has undergone since the previous commit. Here, they are labeled `new file` because they are not found among the currently tracked files of Vms. 

Files may be removed from the staging area as well. This can be achieved using the `vms unstage` command. For example, if we wanted to unstage the `src/bar` file, we would type:
```
$ vms unstage src/bar
``` 

Doing so would remove the file from the index, meaning it will not be included in the next commit. 

Similar to `vms stage`, the `vms unstage` command can also take singular or multiple files and directories as arguments. 

## Committing your changes

When we commit, we are telling Vms that we want to save all of the snapshots in our staging area into the repository. 

The `vms commit` command requires that we provide a message argument to give context to the commit. Surrounding your message by either single or double quotation marks will enable us to write a multi-word message to accompany our commit.

Suppose we left our staging area as it was previously, containing files `README.md`, `CONTRIBUTING.md`, `src/bar`, and `src/foo` in the staging area and we committed with the message `Add source files and README and CONTRIBUTING documents`.

Then, running `vms status` after committing we would receive an output that looks like this:
```
$ vms commit "Add source files and README and CONTRIBUTING documents"
$ vms status
On branch master  [6b12dc]
No changes to staged or tracked files, working tree clean

Sub-directories
    src/
    tests/
```

Based on the above, we see some noteworthy differences our status:
- we are still on the `master` branch, and it has moved to the new commit we have created.
- we no longer see the files we staged and committed under the `Untracked files` heading.
- we are now shown (again) the message `No changes to staged or tracked files, working tree clean`. 

The final point is important. It reflects the side effect that the commit operation adds the staged files to the list of files tracked by Vms. So now Vms will tell us when there have been changes applied to these files that we might want to stage.

### Modifying tracked files

Suppose we changed some of these files now. If we modify the contents of the file `src/foo` and run `vms status` we would receive an output that looks like this:
```
$ vms status
On branch master  [6b12dc]
Changes not yet staged for commit
  (use "vms stage <file>" to update or stage changes to be committed)

    modified:    src/foo

Sub-directories
    src/
    tests/
```

The modifications have been detected and listed in the status output under the heading `Changes not yet staged for commit`. We are also given a suggestion to stage the file so that these changes can be recorded in the repository.

Vms also detects when tracked files have been deleted. For example, if we were to now delete the file `CONTRIBUTIONS.md` and run `vms status`, it would display an output that looks like:
```
$ vms status
On branch master  [6b12dc]
Changes not yet staged for commit
  (use "vms stage <file>" to update or stage changes to be committed)

    deleted:     CONTRIBUTING.md
    modified:    src/foo

Sub-directories
    src/
    tests/
```

We can stage these files to be committed as we had done previously. If we did, and ran `vms status` again, we would see:
```
$ vms stage src/foo CONTRIBUTING.md 
$ vms status
On branch master  [6b12dc]
Changes staged for commit
  (use "vms unstage <file>" to unstage changes to file)
  (use "vms commit <message>" to commit all staged changes)

    deleted:    CONTRIBUTING.md
    modified:     src/foo

Sub-directories
    src/
    tests/
```

Then, we can also commit as we had done previously.
```
$ vms commit "Clean up foo and delete CONTRIBUTING.md"
```
It is worth noting that when when a file deletion is committed, the file gets removed from tracking.

### What exactly are tracked files?

At any point, the files being tracked by Vms are the ones that are being tracked by the commit pointed to by the current branch. Files remain in tracking and will continue to be tracked by future commits unless they are removed.

If we wanted to check the files being tracked by any given commit, we may use the `vms info` command. When using `vms info`, the user must provide the commit id corresponding to the commit they want to inspect. A truncated version of the full 40-character commit id may be provided as long as it uniquely matches a valid commit in the repository. For example, if we wanted to get information about our previous commit, we would type:
```
$ vms info 6b1
```
and receive the output:
```
commit  6b12dc867580240ff31160b3535053ce85096d26
Date    Wed Jun 19 12:27:22 2019
parent  a7ef0e62bf71e5ea3e725394a92e60f4d6c3d894

    Add source files and README and CONTRIBUTING documents

Files tracked in this commit

    CONTRIBUTING.md
    README.md
    src/bar
    src/foo
```

The `vms info` command can also allow you to view a snapshot of a file recorded by the given commit if you provide the command with an additional file argument.

For example, if we wanted to view the contents of `README.md` recorded by the commit above, we may type:
```
$ vms info 6b1 README.md
```

## Browsing the history of your project

We may view your commit history with the `vms log` command. In our example, we might see something like:
```
===
commit  24f734c449fbc5beabfb554d832b860e202b48af
Date    Wed Jun 19 13:53:42 2019
parent  6b12dc867580240ff31160b3535053ce85096d26

    Clean up foo and delete CONTRIBUTING.md

===
commit  6b12dc867580240ff31160b3535053ce85096d26
Date    Wed Jun 19 12:27:22 2019
parent  a7ef0e62bf71e5ea3e725394a92e60f4d6c3d894

    Add source files and README and CONTRIBUTING documents
```

The `vms log` command chronologically displays for every commit: its full commit id, its parent (i.e. predecessor), the date and time it was created, and its associated message.

Just like before, we may use the `vms info` command to inspect these commits.

## Restoring previous versions of your files

One of the most important features of any version control system the maintenance one's revision history so that one can revert to other versions if necessary. So far, we have seen how Vms keeps a history of our changes through commits. Now, we will see how we can retrieve and restore versions of our files using the `vms checkout` command.

The `vms checkout` command is a multipurpose command, but shared among its various applications is the capacity to write to your current working directory.  As a result, this command can be dangerous. In particular, it may result in the loss of uncommitted file changes in the event those files are overwritten by the checkout, so please be careful to commit any changes you would like to preserve before checking out files!

Because the command is dangerous, Vms gives a warning and asks for confirmation before proceeding with the operation.

Checkout has two options for retrieving files. It can checkout every file tracked by the provided commit, or it can checkout only the files we specify. 

For example, if we decided we wanted to undo our most recent changes to `src/foo`, we would use the command:
```
$ vms checkout files 6b1 src/foo
```
Because this command is potentially dangerous, we are prompted for confirmation with an output that looks like:
```
commit  6b12dc867580240ff31160b3535053ce85096d26
Date    Wed Jun 19 12:27:22 2019
parent  a7ef0e62bf71e5ea3e725394a92e60f4d6c3d894

    Add source files and README and CONTRIBUTING documents

Checking out files
    src/foo

Warning: checking out files may overwrite uncommitted changes for these files in the working directory.
Confirm checkout (y/n):
```

Note that the output displays a full list of the files being checked out by the command, so we can consider the safety of the operation before proceeding.

Typing `y` into the prompt would instruct Vms to proceed with the checkout operation, writing or overwriting all files listed under the `Checking out files` header with the versions recorded in the commit to our current working directory. Alternatively, typing `n` into the prompt would abort the operation, leaving our files untouched. 

## Creating and using branches

Branches are a feature that allows users to create and organize independent lines of development inside their repository. Branches may be used to test the waters in the development of new features, fixing of bugs, and perform other revisions in a more convenient manner than if we were to do all of our development sequentially on the `master` branch.

The usefulness of the branching model can perhaps best be illustrated through an example. Suppose we wanted to develop a new feature in our project that would take a substantial amount of work and time to complete. Then, it may be wise to use a separate branch for this development task.

We can create a new branch using the `vms mkbranch` method by providing the name of the branch we want to create. For example, if we wanted to develop the feature in a new branch called `my_feature`, we would create the branch by typing:
```
$ vms mkbranch my_feature
```

This would create a new branch at the same location as our current commit. *The* `vms mkbranch` *command also has an option to create a new branch located at another commit by providing its id as an additional argument.*

We can see the creation of this new branch reflected in the output of `vms status`:
```
On branch master  [24f734]

Other branches
    my_feature  [24f734]

No changes to staged or tracked files, working tree clean

Sub-directories
    src/
    tests/
```

Now that our branch has been created, we may swap to it using the `vms checkout` command:

```
$ vms checkout branch my_feature
```

As stated previously, `vms checkout` can overwrite files in your current working directory, so be careful to avoid the unintentional loss of uncommitted changes. 

Checking out a branch is very similar to checking out all of the files from a commit. The key difference is that it also sets the branch you provided to be the new current branch.

After confirming our checkout, we can use `vms status` to see that we have certainly swapped to the `my_feature` branch:

```
On branch my_feature  [24f734]

Other branches
    master  [24f734]

No changes to staged or tracked files, working tree clean

Sub-directories
    src/
    tests/
```

Great! We may now development as we normally would. Suppose we now make a few changes to our files and commit those changes. If we ran `vms status` after committing, we might see something like:
```
On branch my_feature  [7b7952]

Other branches
    master  [24f734]

No changes to staged or tracked files, working tree clean

Sub-directories
    src/
    tests/
```

Notice that the `my_feature` branch has moved along with the new commit, but `master` has remained exactly where we left it. This quality means that we may continue to develop freely in this branch and create new commits while having convenient access to the point where our development began. This gives us some significant advantages. It means:
- we may easily suspend work on this feature and swap back to the `master` branch (thereby restoring our files) to work on another feature or bug fix that demands our attention and join our changes later with `vms merge`.
- we may even easily abandon our development in this branch by swapping back to the `master` branch and deleting `my_feature` with the `vms rmbranch` command.
- if we complete developing our feature and want to bring our changes into `master`, we can easily join those changes into our `master` branch using the  `vms merge` command.

### Merging branches

When we merge branches, we are combining the versions of the files from a given branch with those in the current branch. The operation is more involved than the others, so only the basics will be discussed and demonstrated in this tutorial.

The simplest execution of merge occurs when the current branch is a direct ancestor of the given branch. As an example of this, consider the previous example in which we committed changes in the branch `my_feature` without moving our `master` branch. In that scenario, the commit pointed to by `master` is a direct ancestor of the one pointed to by `my_feature`.

We could merge the changes made in `my_feature` into `master` by checking out the `master` branch and merging:
```
$ vms checkout branch master
$ vms merge my_feature
```

The result will be a what is known as a "fast-forward merge", in which `master` will move to point to the commit pointed to by `my_feature` and update any files in the current working directory with the modified versions recorded by that commit. Therefore, at the end of the merge, both `master` and our current working directory will be "up-to-date" with all the file changes we have made in the `my_feature` branch.

In comparison, the more complex version of merge occurs when neither branch is a direct ancestor of the other. In this scenario, the updated versions of files from the commits pointed to by both branches are used to create a new commit which the current branch will move to at the conclusion of the operation.

If there are any files that have been updated or created in both branches so that they contain conflicting contents, then a merge conflict will occur and the file will be overwritten with contents that might look like this:
```
<<<<<<< version: master
Here are the copied contents of foo in master. 
=======
Here are the copied contents of foo in my_feature. 
>>>>>>> version: my_feature
```

Merge conflicts must be resolved manually by the user by first modifying the contents of the file as desired (considering the contents in both versions) and then staging and committing the file as normal. 

## Next steps

This tutorial has been a brief introduction to get you up and running with Vms. You have likely noticed the high degree of similarity between Vms and Git-- that is intentional! Vms was designed to provide simple interfaces, features, structures, and syntax that resemble those of Git so that it might serve as a simple alternative or soft introduction to Git.

Though this tutorial has not covered some of the more nuanced behaviors of the commands, you should now be in a position to accomplish the vast majority of any of your version-control requirements with either Vms or Git.

For those who want to learn more about the specifications behind the design of Vms, please see the [command specification](COMMANDS.md). 

Or for anyone who wants to learn more about Git and its more advanced features, I recommend taking a look at [Pro Git](https://git-scm.com/book/en/v2).