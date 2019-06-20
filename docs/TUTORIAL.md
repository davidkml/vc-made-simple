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

Over the course of the tutorial, you will learn how to get started using Vms to:
- Initialize repositories
- Stage and unstage files for commit
- Commit your changes to the repository
- Browse and inspect the history of your files
- Restore prior versions of your files
- Create and join independent branches of development

Because Vms is a command-line tool, some basic familiarity with the command line is necessary. However, even those who are not as comfortable with the command line should be able to follow along conceptually with this tutorial.

For those interested in reading a more complete specification of its behavior and nuances, please take a look at the [design documentation](DESIGN.md). 

## Creating a repository

In the majority of cases, you will be creating repositories when you either:
1. Are beginning a new project which you want to apply version control to, or
2. Wanting to apply version control to an already existing project

In general, to initialize your repository, you will navigate to the root directory of the project you want to manage with Vms and use the command `vms init`.

As a simple example, suppose you have a project you have been developing, and the project lives in a directory with the path `/Users/user/my_project`.

Then, to initialize a Vms repository in your project, you would move into your project's directory with:
```bash
$ cd /Users/user/my_project
```
and type the command:
```
$ vms init
```
This creates a new subdirectory named `.vms` which will hold all of your repository's information. Having initialized your repository, you may now begin using Vms to track versions of your files as you develop your project. 

## Checking the status of your files

You may check the state of your files at any time by typing the command `vms status`.

It is important to understand that at this point immediately following initialization, the repository is not yet tracking any of your files. Like Git, Vms does not begin tracking any of your files unless you explicitly tell it to.

If your project has been newly created and the directory is empty, using `vms status` you might see something that looks like the following:
```
$ vms status
On branch master  [a7ef0e]

No changes to staged or tracked files, working tree clean
```

However, as you add files to the directory (or if files already exist), they will show up in the output of `vms status`. For example, if your project directory contained files named `README.md` and `CONTRIBUTING.md`, as well as subdirectories named `src/` and `tests/` containing their own set of files, they might appear in the status like this:
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

Let's begin deciphering the information displayed to us by `vms status`. Based on the output above we know:
- The name of the current branch as well as a shortened id of the commit at which it is presently located (by default, every repository is initialized with a branch named `master` that initially points to a dummy commit)
- We have a clean working directory, which means that there are currently no modifications to the files being tracked by Vms.
- What files are in our directory that are not currently being tracked. 

As we continue with this tutorial, we will see that, generally, as we begin tracking and modifying files, `vms status` will also provide information about other branches, the staging area, and modifications to tracked files. The command also provides suggestions on what you might want to do, given this information.

## Staging and unstaging files for commit

Before you "commit" (i.e. record) changes of files to your repository, you must add it to the staging area.

The staging area is an intermediate area that serves as an index of all the files you want included in your next commit. When you add a file to the staging area, it also creates a temporary snapshot of the file as it currently exists, though it will not "save" the snapshot unless you commit the file. Thus, in a way, the staging area serves as setting in which you can organize, manage, and review your changes before you commit them to your repository.

Files are staged using the command `vms stage`.  The command can take one or multiple names (paths) of files and directories as arguments. If a directory is given as an argument to `vms stage`, then the command will stage all the files in that directory. You are not required to stage all your files at once; the state of the staging area will persist until you are ready to commit.

For example, suppose we wanted to stage `README.md`, `CONTRIBUTING.md`, and all of the files inside the `src/` subdirectory. Using `vms stage` and then `vms status`, we might receive an output that looks like: 

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

We now see the files moved from being placed under the `Untracked files` heading to the `Changes staged for commit` heading, which denotes that they are in our staging area. We also see the type of modification made to the file. Here, they are labeled `new file` because they were not previously tracked by Vms. 

As hinted by the `vms status` command, files may be removed from the staging area as well. You can achieve this using the `vms unstage` command. For example, if we wanted to unstage the `src/bar` file, we would type:
```
$ vms unstage src/bar
``` 

Doing so would remove the file from the index meaning it will not be included in the next commit. This removal would be reflected in `vms status`. 

Like stage, unstage can also take one or multiple files and directories as arguments. 

## Committing your changes

When you commit, you are telling Vms that you want to save all the snapshots you have indexed in your staging area into the repository. 

The `vms commit` command requires that the user provide a message as an argument. Surrounding your message by either single or double quotation mark will enable you to write a multi-word message to accompany your commit.

With our staging area as we left it previously, with files `README.md`, `CONTRIBUTING.md`, `src/bar`, and `src/foo` in the staging area, suppose we were to commit with the message `Add source files and README and CONTRIBUTING documents`.

Running `vms status` again after committing we might receive an output that looks like this:
```
$ vms commit "Add source files and README and CONTRIBUTING documents"
$ vms status
On branch master  [6b12dc]
No changes to staged or tracked files, working tree clean

Sub-directories
    src/
    tests/
```

Some differences worth noting in our status outputs include:
- We are still on the `master` branch, and it has moved to the new commit we have created.
- We no longer see our files under the `Untracked files` heading
- We  see the message `No changes to staged or tracked files, working tree clean` once again. 

The final point is important: the commit operation adds the staged files to tracking so that we will know when there have been modifications we might want to stage.

### Modifying tracked files

Suppose we changed some of these files now. If we modify the contents of the file `src/foo` and run `vms status` we might receive an output that looks like this:
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

The modifications have been detected by Vms and listed in the status output under the heading `Changes not yet staged for commit`, and we are prompted to stage the file so that these changes can be recorded in the repository.

Vms also detects when tracked files have been deleted. For example, if we were to now delete the file `CONTRIBUTIONS.md` and run `vms status`, we might receive an output that looks like:
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

We can stage these files to be committed as we had done previously. Then, running `vms status` again, we would see:

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

We can then also commit as we have done previously.
```
$ vms commit "Clean up foo and delete CONTRIBUTING.md"
```
It is worth noting that when when a file deletion is committed, the file gets removed from tracking.

### What are tracked files

At any point, the files being tracked by Vms are the ones that are being tracked by the commit pointed to by the current branch. Files remain in tracking (and will continue to be tracked by future commits) unless they are removed.

If you wanted to check the files being tracked by any given commit, you may use the `vms info` command. When using `vms info`, the user must provide the commit id associated with the commit they want to inspect. A shortened version of the commit id may be provided so long as it uniquely matches a valid commit in the repository. For example, if we wanted to get information on our previous commit, we would 
type:
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

The `vms info` command can also allow you to view the contents of a file as was recorded by the given commit if you provide it an additional filename as an argument.

For example, if we wanted to view the contents of README.md as was recorded by the commit above, we may type:
```
$ vms info 6b1 README.md
```

## Browsing the history of your project

You may view your commit history with the `vms log` command. In our example, we might see something like:
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

The `vms log` command displays chronologically the full commit id of every commit , its parent (i.e. predecessor), date and time it was created, and associated message.

Just like before, you may use the `vms info` command to inspect these commits.

## Restoring previous versions of your files

One of the most important features of any version control system is the ability to keep a history of your changes so you can revert back to them if necessary. So far, we have seen how Vms keeps a history of your changes through commits. Now, we will see how we can restore previous versions of our files using the `vms checkout` command.

The `vms checkout` command is a multipurpose command, but common to its various uses is the capacity to write or overwrite files in your current working directory with versions recorded in a given commit.  As a result, this command can be dangerous. Most notably, it may cause you to lose uncommitted changes to your files in the event they are overwritten, so be sure to commit any changes you want to keep before checking out files!

Because the command is dangerous, Vms provides a warning and asks for confirmation before proceeding with the checkout operation.

Checkout has two options for checking out files. It can checkout all files tracked by the provided commit, or it can checkout only select files. 

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

Typing `y` into the prompt would instruct Vms to proceed with the checkout operation, writing or overwriting all files listed under the `Checking out files` header with the versions recorded in the commit to our current working directory. Alternatively, typing `n` would abort the operation, leaving our files untouched. 

## Creating and using branches

Breanches are a feature that allows users to create and organize independent lines of development inside their repository. They might be used to test the waters in the development of new features, fixing of bugs, and other revisions in a more convenient manner than if you were to do all of your development sequentially on the `master` branch.

The usefulness of the branching model can perhaps best be illustrated through an example. Suppose we wanted to develop a new feature in our project that would take a substantial amount of work and time to complete. Then, it may be wise to use a separate branch for this development task.

We can create a new branch using the `vms mkbranch` method by providing the name of the branch we want to create. For example, if we had wanted to develop the feature in a new branch called `my_feature`, we would create the branch by typing:
```
$ vms mkbranch my_feature
```

This would create a new branch at the same location of our current commit. The `vms mkbranch` command also has an option to create a new branch located at another commit by providing an associated commit id.

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

As stated previously, a characteristic of `vms checkout` is that it can overwrite files in your current working directory, so be careful in its use to avoid unintentionally losing uncommitted changes. 

Checking out a branch is very similar to checking out all files from a commit. The key difference is that it also sets the branch you provided to be the new current branch.

After confirming our checkout, we can use `vms status` to see that we have indeed swapped to this branch:

```
On branch my_feature  [24f734]

Other branches
    master  [24f734]

No changes to staged or tracked files, working tree clean

Sub-directories
    src/
    tests/
```

Great! We may now begin our development. Suppose we make a few changes to our files and commit those changes. If we ran `vms status` after committing, we might see something like:
```
On branch my_feature  [7b7952]

Other branches
    master  [24f734]

No changes to staged or tracked files, working tree clean

Sub-directories
    src/
    tests/
```

Notice that `my_feature` has moved along with the new commit, but `master` remains exactly where we left it. This quality means that we may continue to develop freely in this branch and create new commits while having easy access to the point where our development began. This gives us some significant advantages:
- we may easily suspend work on this feature and swap back to the `master` branch (thereby restoring your files) to work on another feature or bug fix that demands our attention and join our changes later with `vms merge`.
- we may even easily abandon your development in this branch if we decide it is not worth continuing by swapping back to the `master` branch and deleting `my_feature` with the `vms rmbranch` command.
- If we do complete your feature and want to bring our changes into `master`, we can easily join those changes into our `master` branch using the  `vms merge` command.

### Merging branches

When you merge branches, you are combining the versions of the files from a given branch with those in the current branch. The operation is more involved than others, so only the basics will be discussed in this tutorial.

The simplest execution of merge occurs when the current branch is a direct ancestor of the given branch. As an example of this, consider the previous case in which we committed changes in the branch `my_feature` without moving our `master` branch. In that scenario, the commit pointed to by `master` is a direct ancestor of the one pointed to by `my_feature`.

We could merge the changes made in `my_feature` into `master` by checking out the `master` branch and merging:
```
$ vms checkout branch master
$ vms merge my_feature
```

The result will be a what is known as a "fast-forward merge", in which `master` will move to point to the commit pointed to by `my_feature` and update any files with the modified versions recorded by that commit. Therefore, at the end of the merge, `master` will be "up-to-date" with all the file changes you have made in the `my_feature` branch

In comparison, the more complex version of merge occurs when neither branch is a direct ancestor of the other. In this scenario, the updated versions of files from the commits pointed to by both branches are used to create a new commit which the current branch will move to at the conclusion of the operation.

If there are any files that have been updated or created containing conflicting contents in both branches, then they may result in merge conflicts. The file will be overwritten with contents of  that might look like this:
```
<<<<<<< version: master
Here are the copied contents of foo in master. 
=======
Here are the copied contents of foo in my_feature. 
>>>>>>> version: my_feature
```

Merge conflicts must be resolved manually by you, the user, by first modifying the contents of the file to your liking (considering the contents in both versions) before staging and committing the file as normal. 

## Next steps

This has been a brief introduction to get you up and running with Vms. You have likely noticed the high degree of similarity between Vms and Git-- that is intentional! The goal for Vms was to provide simple interfaces, features, structures, and syntax that resemble Git so that it might serve as a simple alternative or soft introduction to Git.

Though this tutorial has not covered the nuances of all the commands in full detail, you should now be in a position to accomplish the vast majority of any of your version-control requirements with either Vms or Git.

For those who want to learn more about the specifications, behaviors, and decisions behind the design of Vms, please see the [design documentation](DESIGN.md). 

Or for anyone who wants to learn more about Git and its more advanced features, I recommend taking a look at [Pro Git](https://git-scm.com/book/en/v2).