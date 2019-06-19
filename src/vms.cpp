#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/dir.h>

#include <map>
#include <boost/serialization/map.hpp>

#include <set>
#include <unordered_set>
#include <list>
#include <stack>
#include <queue>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/stack.hpp>

#include <sstream>
#include <fstream>

#include "vms.hpp"
#include "utils.h"
#include "archive.hpp"
#include "commit.hpp"
#include "blob.hpp"
#include "access.hpp"


using namespace std;

const string STAGE_DELETE = "DELETED";

enum RelativeFileStatus {
    NEW,
    MODIFIED,
    UNMODIFIED,
    DELETED,
    NOT_FOUND
};

int move_from_cache_to_objects(const string& id) {
    ostringstream cache_path;
    cache_path << ".vms/cache/" << id;

    // Create subdirectory if it does not already exist and move
    ostringstream objects_path;

    string id_prefix;
    string id_suffix;
    split_prefix_suffix(id, id_prefix, id_suffix, PREFIX_LENGTH);

    objects_path << ".vms/objects/" << id_prefix;
    mkdir(objects_path.str().c_str(), 0755);
    
    objects_path << "/" << id_suffix;
    int ret = move_file(cache_path.str().c_str(), objects_path.str().c_str());

    if (ret != 0) {
        cerr << "Error occurred: unable to move staged changes from cache to objects directory" << endl;
        return -1;
    }

    ret = chmod(objects_path.str().c_str(), 0444);
    if (ret != 0) {
        cerr << "Error occurred: unable to change permissions of blob file." << objects_path.str() << endl;
        return -1;
    }

    return 0;
}

/** Helper method for restoring a commit from a shortened commit id
 * Utilization assumes prior validation that shortened id is valid, meaning:
 * - it is longer than the defined PREFIX_LENGTH
 * - its prefix matches a subdirectory in the .vms/objects directory
 * - its suffix matches exactly one file in the .vms/objects/<prefix> subdirectory **/
int restore_commit_from_shortened_id(const char* commit_id, Commit& commit) {
    string id_prefix;
    string id_suffix;
    split_prefix_suffix(string(commit_id), id_prefix, id_suffix, PREFIX_LENGTH);
    
    ostringstream full_id;
    full_id << id_prefix;

    ostringstream obj_path;
    obj_path << ".vms/objects/" << id_prefix;

    const char* dirpath = obj_path.str().c_str();

    if (is_valid_dir(dirpath)) {
    
        DIR *dirptr = opendir(dirpath);
        struct dirent *entry = readdir(dirptr);

        while (entry != NULL) {
            if (strcmp(".", entry->d_name) != 0 && strcmp("..", entry->d_name) != 0) {
                if (strncmp(id_suffix.c_str(), entry->d_name, id_suffix.length()) == 0) {

                    obj_path << "/" << entry->d_name;
                    full_id << entry->d_name;

                }
            }
            entry = readdir(dirptr);
        }
    }
    restore<Commit>(commit, obj_path.str());

    // verify no tampering or corruption of restored object
    if (commit.hash() != full_id.str()) {
        cerr << "Fatal error has occurred in retrieval of commit: uuid mismatch. Archived object may have been corrupted. Exiting..." << endl;
        return -1;
    }

    return 0;
}

int restore_blob_from_full_id(const string& blob_id, Blob& blob) {
    string blob_id_prefix;
    string blob_id_suffix;
    split_prefix_suffix(blob_id, blob_id_prefix, blob_id_suffix, PREFIX_LENGTH);

    ostringstream blob_path;
    blob_path << ".vms/objects/" << blob_id_prefix << "/" << blob_id_suffix;

    restore<Blob>(blob, blob_path.str());

    // verify no tampering or corruption of restored object
    if (blob.hash() != blob_id) {
        cerr << "Fatal error has occurred in retrieval of file contents: uuid mismatch. Archived object may have been corrupted. Exiting..." << endl;
        return -1;
    }

    return 0;
}

/** Creates directory path to file if it doesn't already exist and returns position of the end slash, or 0 if given file is not in a subdirectory.
 * Input: .vms/objects/de/<file> 
 * Output: 15
 * 
 * Input: foo
 * Output: 0 **/
int create_directory_path(string filepath) {
    int pos = 0;
    size_t ret = 0;
    while(ret != string::npos) {
        pos = ret;
        ret = filepath.find("/", pos+1);
        if (pos != 0) {
            mkdir(filepath.substr(0, pos+1).c_str(), 0755);
        }
    }
    return pos;
}

 
int find_split_point(const string& branch_A, const string& branch_B, string& strbuf) {
    /** Design notes:
     * Algorithm using breadth-first search to find split point, ending as soon as common ancestor is found. Finds split point closest relative to both sources.
     * This algorithm does not guarantee it will identify one source as a direct ancestor of the other (e.g. opportunity for fast-forward merge).
     * To achieve this alternative result, may implement algorithm that searches the entire commit graph which is considerably more costly.
     * 
     * Cases in which this algorithm will find "suboptimal" split point are expected to be relatively uncommon, so this partial search is sufficient in most cases.
     * In cases for which "suboptimal" split points are found, it may result in "false positive" merge conflicts.
    */

    string id_A;
    string id_B;

    if (get_id_from_branch(branch_A, id_A) != 0) {
        return -1;
    }

    if (get_id_from_branch(branch_B, id_B) != 0) {
        return -1;
    }

    if (id_A == id_B) { // both branches point to same commit, so trivially found
        strbuf = id_A;
        return 0;
    }

    unordered_set<string> seen_A;
    unordered_set<string> seen_B;
    unordered_set<string> seen_union;
    queue<string> fringe;
    pair<unordered_set<string>::iterator,bool> ret;

    seen_A.insert(id_A);
    seen_B.insert(id_B);
    seen_union.insert(id_A);
    seen_union.insert(id_B);
    fringe.push(id_A);
    fringe.push(id_B);

    string current_id;
    Commit current_commit;
    pair<string, string> parents;

    while (!fringe.empty()) {
        current_id = fringe.front();
        fringe.pop();

        restore_commit_from_shortened_id(current_id.c_str(), current_commit);
        
        parents = current_commit.parent_ids();

        if (seen_A.find(current_id) != seen_A.end()) {  // if current id was first seen from source A

            if (!parents.first.empty() && seen_A.insert(parents.first).second) { // if first parent is not empty string and has not been seen before from source A (side effect: adds to seem_A if true: not seen before, does nothing if has seen before)

                fringe.push(parents.first);
                ret = seen_union.insert(parents.first);

                if (!ret.second) {  // if ret.second == false, then insertion failed. Found split point. break and return.
                    strbuf = parents.first;
                    break;
                }

            } 
            
            if (!parents.second.empty() && seen_A.insert(parents.second).second) {

                fringe.push(parents.second);
                ret = seen_union.insert(parents.second);

                if (!ret.second) {
                    strbuf = parents.second;
                    break;
                }
            }

        } else {  // current id was first seen from source B

            if (!parents.first.empty() && seen_B.insert(parents.first).second) { // if first parent is not empty string and has not been seen before from source B (side effect: adds to seem_B if true: not seen before, does nothing if has seen before)

                fringe.push(parents.first);
                ret = seen_union.insert(parents.first);

                if (!ret.second) {  // if ret.second == false, then insertion failed. Found split point. break and return.
                    strbuf = parents.first;
                    break;
                }

            } 
            
            if (!parents.second.empty() && seen_B.insert(parents.second).second) {

                fringe.push(parents.second);
                ret = seen_union.insert(parents.second);

                if (!ret.second) {
                    strbuf = parents.second;
                    break;
                }
            }


        }
    }

    return 0;

}

RelativeFileStatus find_relative_file_status(string filename, const map<string, string>& x, const map<string, string>& ref) {
    map<string, string>::const_iterator x_entry = x.find(filename);
    map<string, string>::const_iterator ref_entry = ref.find(filename);

    bool present_in_x = x_entry != x.end();
    bool present_in_ref = ref_entry != ref.end();

    if (present_in_x && !present_in_ref) {
        return NEW;
    }

    if (!present_in_x && present_in_ref) {
        return DELETED;
    }

    if (present_in_x && present_in_ref) {
        if (x_entry->second == ref_entry->second) {
            return UNMODIFIED;
        }

        return MODIFIED;

    }

    return NOT_FOUND;
}

bool has_relative_changes(const map<string, string>& x, const map<string, string>& ref) {
    if (x.empty()) {
        return false;
    }

    map<string, string>::const_iterator x_entry;

    for (x_entry = x.begin(); x_entry != x.end(); x_entry++) {
        if (find_relative_file_status(x_entry->first, x, ref) != UNMODIFIED) {
            return true;
        }
    }

    return false;
    
}


int vms_init() {

    char cwd_buf[PATH_MAX];

    if (getcwd(cwd_buf, PATH_MAX) == NULL) {
        cout << "Error occurred in retrieving path to current working directory: " << strerror(errno) << endl;
        return -1;
    }

    // Initialize directories
    int ret = make_dir(".vms");
    if (ret != 0) {
        return -1;
    }

    ret = make_dir(".vms/objects");
    if (ret != 0) {
        return -1;
    }

    ret = make_dir(".vms/branches");
    if (ret != 0) {
        return -1;
    }

    ret = make_dir(".vms/cache");
    if (ret != 0) {
        return -1;
    }

    // Initialize files
    map<string, string> index;
    save< map<string, string> >(index, ".vms/index");

    stack<string> log;
    save< stack<string> >(log, ".vms/log");

    // Initialize initial commit
    Commit sentinal;
    string sentinal_id = sentinal.hash();

    create_and_write_file(".vms/HEAD", "master", 0644);
    create_and_write_file(".vms/branches/master", sentinal_id.c_str(), 0644);

    ostringstream obj_path;

    string sentinal_id_prefix;
    string sentinal_id_suffix;
    split_prefix_suffix(sentinal_id, sentinal_id_prefix, sentinal_id_suffix, PREFIX_LENGTH);

    obj_path << ".vms/objects/" << sentinal_id_prefix;
    make_dir(obj_path.str().c_str());
    obj_path << "/" << sentinal_id_suffix;

    save<Commit>(sentinal, obj_path.str());

    cout << "Repository initialized at " << cwd_buf << "\n";

    return 0;
}

int vms_stage(const char* filepath) {

    // Load index
    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");

    // if file was previously being tracked but is now deleted
    if(is_tracked_file(filepath) && !is_valid_file(filepath)) {
        // Puts filepath into index map with special string, save the updated index, and return
        index[filepath] = STAGE_DELETE;
        save< map<string, string> >(index, ".vms/index");
        return 0;
        
    } else if (is_staged_file(filepath) && !is_valid_file(filepath)) { // if file was previously staged but is now deleted
        // Remove the file from index, save the updated index, and return
        index.erase(string(filepath));
        save< map<string, string> >(index, ".vms/index");
        return 0;

    } 

    // Blob the file's contents and get its id
    ifstream ifs(filepath);
    if (!ifs.is_open()) {
        cerr << "Error occurred: unable to open file " << filepath << " for staging" << endl;
        return -1;
    }
    Blob file(ifs);

    string file_id = file.hash();

    // Puts filepath and id into the index map and save the updated index
    index[filepath] = file_id;
    save< map<string, string> >(index, ".vms/index");

    // Save blob in cache
    ostringstream cache_path;
    cache_path << ".vms/cache/" << file_id;
    save<Blob>(file, cache_path.str());

    return 0;
}

int vms_unstage(const char* filepath) {
    // Load index
    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");

    // remove entry from the index map and save the updated index
    index.erase(string(filepath));
    save< map<string, string> >(index, ".vms/index");

    // Note: decide to not remove cache because unnecessary: will clear cache after commits
    
    return 0;
}

int vms_commit(const char* msg) {
    // Load index
    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");

    // Create new commit and get its map, which is currently identical to its parent's map
    Commit commit(msg);

    map<string, string> commit_map = commit.get_map();
    
    // Check if any tracked changes to commit, if not print and return, otherwise continue
    if (!has_relative_changes(index, commit_map)) {
        cerr << "No changes staged to commit" << endl;
        return -1;
    }

    // Update commit's internal map, and move files from cache to objects directory

    set<string> uuid_set;     // Create set to track which uuids have been seen before so don't try to move twice
    pair<set<string>::iterator,bool> insert_ret;

    map<string,string>::iterator it;
    for (it=index.begin(); it!=index.end(); ++it) {
        // if staged is file mapped to STAGE_DELETE string, remove it from commit tree
        if (it->second == STAGE_DELETE) {
            commit.remove_from_map(it->first);
        } else { // puts entry to commit map and move files from cache to objects directory
            commit.put_to_map(it->first, it->second);
            insert_ret = uuid_set.insert(it->second);
            if (insert_ret.second) { // if insert succeed, then uuid has not been seen yet and can be moved.
                move_from_cache_to_objects(it->second);
            }
        }
    }

    // Iterate through cache directory, clearing it
    DIR *dirptr = opendir(".vms/cache");
    struct dirent *entry = readdir(dirptr);
    
    char cache_path[BUFSIZ];
    
    while (entry != NULL) {

        sprintf(cache_path, "%s/%s", ".vms/cache", entry->d_name);

        if (is_valid_file(cache_path)) {
            remove_file(cache_path);
        }

        entry = readdir(dirptr);

    }

    // Clear index and save it
    index.clear();
    save< map<string, string> >(index, ".vms/index");

    // Change position of branch pointed to by HEAD
    string commit_id = commit.hash();
    
    string branch_path;
    if (get_branch_path(branch_path) != 0) {
        return -1;
    }
    
    create_and_write_file(branch_path.c_str(), commit_id.c_str(), 0644);

    //Push formatted commit string to log and save
    stack<string> log;
    restore< stack<string> >(log, ".vms/log");

    log.push(commit.log_string());
    save< stack<string> >(log, ".vms/log");

    // Serialize and save your commit.
    ostringstream obj_path;

    string commit_id_prefix;
    string commit_id_suffix;
    split_prefix_suffix(commit_id, commit_id_prefix, commit_id_suffix, PREFIX_LENGTH);

    obj_path << ".vms/objects/" << commit_id_prefix;
    make_dir(obj_path.str().c_str());
    obj_path << "/" << commit_id_suffix;

    save<Commit>(commit, obj_path.str());

    // Change permissions of generated file.
    int ret = chmod(obj_path.str().c_str(), 0444);
    if (ret != 0) {
        cerr << "Error occurred: failed to change permissions of commit file." << obj_path.str() << endl;
        return -1;
    }

    return 0;
}

int vms_log() {

    stringstream log_output;

    stack<string> log;
    restore< stack<string> >(log, ".vms/log");
    while (!log.empty()) {
        log_output << "===\n";
        log_output << log.top() << endl;
        log.pop();
    }

    cout << log_output.rdbuf();

    return 0;
}

int vms_status(const char* arg0) {

    string parent_id; 

    if (get_parent_ref(parent_id) != 0) {
        return -1;
    }

    // List branches that currently exist and mark current branch
    string current_branch;
    
    if (get_branch(current_branch) != 0) {
        return -1;
    }

    string branch_id;

    if (get_id_from_branch(current_branch, branch_id) != 0) {
        return -1;
    }

    // Load parent commit
    Commit parent_commit;

    ostringstream parent_path;

    string parent_id_prefix;
    string parent_id_suffix;
    split_prefix_suffix(parent_id, parent_id_prefix, parent_id_suffix, PREFIX_LENGTH);

    parent_path << ".vms/objects/" << parent_id_prefix << "/" << parent_id_suffix;

    restore<Commit>(parent_commit, parent_path.str());
    if (parent_commit.hash() != parent_id) {
        std::cerr << "Fatal error has occurred in retrieval of commit: uuid mismatch. Archived object may have been corrupted. Exiting..." << std::endl;
        return -1;
    }


    // Get copy of map of parent commit
    map<string, string> parent_map = parent_commit.get_map();


    stringstream status_stream;
    status_stream << "On branch " << current_branch.c_str() << "  [" << branch_id.substr(0,6) << "]\n";

    DIR *branch_dirptr = opendir(".vms/branches");
    struct dirent *branch_entry = readdir(branch_dirptr);

    set<string> branches;

    while (branch_entry != NULL) {

        if (strcmp(".", branch_entry->d_name) != 0 && strcmp("..", branch_entry->d_name) != 0) {

            if (strcmp(current_branch.c_str(), branch_entry->d_name) != 0) {
                branches.insert(string(branch_entry->d_name));
            }

        }
        branch_entry = readdir(branch_dirptr);
    }

    set<string>::iterator ss_iter;
    
    if (!branches.empty()) {
        status_stream << "\nOther branches\n";
        for (ss_iter = branches.begin(); ss_iter != branches.end(); ss_iter++) {
            get_id_from_branch(*ss_iter, branch_id);
            status_stream << "    " << *ss_iter << "  [" << branch_id.substr(0,6) << "]\n";
        }
        status_stream << endl;
    }

    // List all files currently staged. (and list type of modification: modified, deleted)
    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");
    map<string,string>::iterator it;

    bool staged_changes = false;
    RelativeFileStatus rf_status;

    for (it = index.begin(); it != index.end(); ++it) {

        rf_status = find_relative_file_status(it->first, index, parent_map);

        if (it->second == STAGE_DELETE) {
            if (!staged_changes) {
                staged_changes = true;
                status_stream << "Changes staged for commit\n";
                status_stream << "  (use \"" << arg0 << " unstage <file>\" to unstage changes to file)\n";
                status_stream << "  (use \"" << arg0 << " commit <message>\" to commit all staged changes)\n\n";
            }

            status_stream << "    deleted:    " << it->first << "\n";

        } else if (rf_status == NEW) {
            if (!staged_changes) {
                staged_changes = true;
                status_stream << "Changes staged for commit\n";
                status_stream << "  (use \"" << arg0 << " unstage <file>\" to unstage changes to file)\n";
                status_stream << "  (use \"" << arg0 << " commit <message>\" to commit all staged changes)\n\n";
            }

            status_stream << "    new file:    " << it->first << "\n";

        } else if (rf_status == MODIFIED) {
            if (!staged_changes) {
                staged_changes = true;
                status_stream << "Changes staged for commit\n";
                status_stream << "  (use \"" << arg0 << " unstage <file>\" to unstage changes to file)\n";
                status_stream << "  (use \"" << arg0 << " commit <message>\" to commit all staged changes)\n\n";
            }

            status_stream << "    modified:     " << it->first << "\n";

        }
        
    }
    if (staged_changes) {
        status_stream << endl;
    }

    // List all files that have been staged and have been modified since staging (and the type of modification)

    bool unstaged_changes = false;
    for (it = index.begin(); it != index.end(); ++it) {

        if (it->second != STAGE_DELETE) {

            if (!is_valid_file(it->first.c_str())) {    // if previously staged file has been deleted, list it as deleted
                if (!unstaged_changes) {
                    unstaged_changes = true;
                    status_stream << "Changes not yet staged for commit\n";
                    status_stream << "  (use \"" << arg0 << " stage <file>\" to update or stage changes to be committed)\n\n";
                }

                status_stream << "    deleted:     " << it->first << "\n";
            } else if (!file_hash_equal_to_working_copy(it->first, it->second)) {    // if tracked file has been modified, list it as modified
                if (!unstaged_changes) {
                    unstaged_changes = true;
                    status_stream << "Changes not yet staged for commit\n";
                    status_stream << "  (use \"" << arg0 << " stage <file>\" to update or stage changes to be committed)\n\n";
                }

                status_stream << "    modified:    " << it->first << "\n";
            }

        } else { // File staged as deleted
            if (is_valid_file(it->first.c_str())) { // if file is no longer deleted and can be staged to be added again, should notify user. By definition, file staged to be deleted from tracking is already tracked, so check if it has changes. If so, then list it as modified
                if (!unstaged_changes) {
                    unstaged_changes = true;
                    status_stream << "Changes not yet staged for commit\n";
                    status_stream << "  (use \"" << arg0 << " stage <file>\" to update or stage changes to be committed)\n\n";
                }

                status_stream << "    modified:    " << it->first << "\n";
            }
        }
    }

    // list all tracked files that have not been staged and have been modified (and the type of modification)


    for (it = parent_map.begin(); it != parent_map.end(); ++it) {
        if (!is_staged_file(it->first.c_str())) {
            if (!is_valid_file(it->first.c_str())) { // unstaged tracked file has been deleted from working directory
                if (!unstaged_changes) {
                    unstaged_changes = true;
                    status_stream << "Changes not yet staged for commit\n";
                    status_stream << "  (use \"" << arg0 << " stage <file>\" to update or stage changes to be committed)\n\n";
                }

                status_stream << "    deleted:     " << it->first << "\n";
            
            } else if (is_modified_tracked_file(it->first.c_str())) {
                if (!unstaged_changes) {
                    unstaged_changes = true;
                    status_stream << "Changes not yet staged for commit\n";
                    status_stream << "  (use \"" << arg0 << " stage <file>\" to update or stage changes to be committed)\n\n";
                }

                status_stream << "    modified:    " << it->first << "\n";
            
            }
        }
    }

    if (!unstaged_changes && !staged_changes) {
        status_stream << "No changes to staged or tracked files, working tree clean\n\n";
    } else if (unstaged_changes) {
        status_stream << endl;
    }

    // list all untracked files in this directory
    set<string> dirs;
    set<string> untracked_files;

    DIR *root_dirptr = opendir(".");
    struct dirent *root_entry = readdir(root_dirptr);


    while (root_entry != NULL) {
        if (is_valid_dir(root_entry->d_name) && strcmp(".", root_entry->d_name) != 0 && strcmp("..", root_entry->d_name) != 0 && strcmp(".vms", root_entry->d_name) != 0) {

            dirs.insert(string(root_entry->d_name));
        
        } else if (is_valid_file(root_entry->d_name) && !is_tracked_file(root_entry->d_name) && !is_staged_file(root_entry->d_name)) {
        
            untracked_files.insert(string(root_entry->d_name));
        
        }
        root_entry = readdir(root_dirptr);

    }

    if (!untracked_files.empty()) {
        status_stream << "Untracked files\n";
        status_stream << "  (use \"" << arg0 << " stage <file>\" to include file to be committed and tracked)\n\n";
        for (ss_iter = untracked_files.begin(); ss_iter != untracked_files.end(); ss_iter++) {
            status_stream << "    " << *ss_iter << "\n";
        }
        status_stream << endl;
    }


    if (!dirs.empty()) {
        status_stream << "Sub-directories\n";
        for (ss_iter = dirs.begin(); ss_iter != dirs.end(); ss_iter++) {
            status_stream << "    " << *ss_iter << "/\n";
        }
        status_stream << endl;
    }
    
    cout << status_stream.rdbuf();
    return 0;
}


int vms_mkbranch(const char* branchname) {
    string src_path;

    if (get_branch_path(src_path) != 0) {
        return -1;
    }

    ostringstream branch_path_stream;
    branch_path_stream << ".vms/branches/" << branchname;

    ifstream src(src_path);
    ofstream dst(branch_path_stream.str());

    dst << src.rdbuf();
    
    src.close();
    dst.close();

    cout << "New branch " << branchname << " created at current location " << endl;

    return 0;
}

int vms_mkbranch(const char* branchname, const char* commit_id) {
    ostringstream branch_path_stream;
    branch_path_stream << ".vms/branches/" << branchname;


    // get the full commit_id and write it to dst
    std::string id_prefix;
    std::string id_suffix;
    split_prefix_suffix(std::string(commit_id), id_prefix, id_suffix, PREFIX_LENGTH);
    
    ostringstream full_id;
    full_id << id_prefix;

    ostringstream dirpath_stream;
    dirpath_stream << ".vms/objects/" << id_prefix;

    const char* dirpath = dirpath_stream.str().c_str();

    if (is_valid_dir(dirpath)) {
    
        DIR *dirptr = opendir(dirpath);
        struct dirent *entry = readdir(dirptr);

        while (entry != NULL) {
            if (strcmp(".", entry->d_name) != 0 && strcmp("..", entry->d_name) != 0) {
                if (strncmp(id_suffix.c_str(), entry->d_name, id_suffix.length()) == 0) {

                    full_id << entry->d_name;

                }
            }
            entry = readdir(dirptr);
        }
    }
    
    create_and_write_file(branch_path_stream.str().c_str(), full_id.str().c_str(), 0644);

    cout << "New branch " << branchname << " created at commit " << full_id.str() << endl;

    return 0;
}

int vms_rmbranch(const char* branchname) {
    ostringstream branch_path;
    branch_path << ".vms/branches/" << branchname;

    remove_file(branch_path.str().c_str());

    return 0;
}

int vms_info(const char* commit_id) {
    Commit commit;
    restore_commit_from_shortened_id(commit_id, commit);
    cout << commit.log_string() << "\n";
    cout << commit.tracked_files_string() << "\n";
    return 0;

}

int vms_info(const char* commit_id, const char* filename) {
    Commit commit;
    restore_commit_from_shortened_id(commit_id, commit);

    // verify file is tracked in this commit and load its blob if it is.
    map<string, string>::iterator it;
    
    if (!commit.find_in_map_and_get_iter(string(filename), it)) {
        cerr << "File not found in commit " << commit_id << endl;
        return -1;
    }

    Blob file;
    restore_blob_from_full_id(it->second, file);

    cout << file.get_content() << endl;

    return 0;

}

int vms_checkout_branch(const char* branchname) {
    // Ask user to verify thay want to checkout the branch.
    string input;
    cout << "Checking out branch " << branchname << "...\n";
    cout << "Warning: checking out branch will overwrite all uncommitted changes in the current branch.\n"; 
    cout << "Confirm checkout (y/n):";
    getline(cin, input);

    while (input != "y" && input != "n") {
        cout << "Please enter y or n:";
        getline(cin, input);
    }

    if (input == "n") {
        cout << "Aborting checkout..." << endl;
        return -1;
    }
    string commit_id;
    if (get_id_from_branch(branchname, commit_id) != 0) {
        return -1;
    }

    if (vms_checkout_files(commit_id.c_str()) != 0) {
        return -1;
    }

    // check HEAD to point to this branch
    create_and_write_file(".vms/HEAD", branchname, 0644);

    // clear staging area
    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");
    index.clear();
    save< map<string, string> >(index, ".vms/index");

    return 0;

}

int vms_checkout_files(const char* commit_id) {
    Commit commit;
    restore_commit_from_shortened_id(commit_id, commit);

    map<string, string>::iterator m_elem;
    map<string, string> commit_map = commit.get_map();

    // Ask user to verify they want to checkout these files.
    cout << commit.log_string() << "\n";
    cout << "Checking out files\n";
    for (m_elem = commit_map.begin(); m_elem != commit_map.end(); m_elem++) {
        cout << "    " << m_elem->first << "\n";
    }
    cout << endl;

    string input;
    cout << "Warning: checking out files may overwrite uncommitted changes for these files in the working directory.\n";
    cout << "Confirm checkout (y/n):";
    getline(cin, input);

    while (input != "y" && input != "n") {
        cout << "Please enter y or n:";
        getline(cin, input);
    }

    if (input == "n") {
        cout << "Aborting checkout..." << endl;
        return -1;
    }

    // User answered "y", so checkout files.
    for (m_elem = commit_map.begin(); m_elem != commit_map.end(); m_elem++) {
        
        create_directory_path(m_elem->first);

        Blob file;
        restore_blob_from_full_id(m_elem->second, file);

        ofstream ofs(m_elem->first);
        ofs << file.get_content();
        ofs.close();
    }

    return 0;

}

int vms_checkout_files(const char* commit_id, const int argc, char* const argv[]) {
    Commit commit;
    restore_commit_from_shortened_id(commit_id, commit);

    // Validate all files to find ones that exist
    map<string, string>::iterator m_elem;
    list<string> found_files;
    for (int i = 4; i < argc; i++) {
        if (commit.find_in_map_and_get_iter(string(argv[i]), m_elem)) {
            found_files.push_back(m_elem->first);
        }
    }

    if (found_files.empty()) {
        cout << "No files match those provided. Exiting..." << endl;
        return -1;
    }

    // Ask user to verify that they want to checkout these files.
    list<string>::iterator l_elem;
    cout << commit.log_string() << "\n";
    cout << "Checking out files\n";
    for (l_elem = found_files.begin(); l_elem != found_files.end(); l_elem++) {
        cout << "    " << *l_elem << "\n";
    }
    cout << endl;

    string input;
    cout << "Warning: checking out files may overwrite uncommitted changes for these files in the working directory.\n";
    cout << "Confirm checkout (y/n):";
    getline(cin, input);

    while (input != "y" && input != "n") {
        cout << "Please enter y or n:";
        getline(cin, input);
    }

    if (input == "n") {
        cout << "Aborting checkout..." << endl;
        return -1;
    }

    // User answered "y", so checkout files.
    for (l_elem = found_files.begin(); l_elem != found_files.end(); l_elem++) {
        
        commit.find_in_map_and_get_iter(*l_elem, m_elem);
        create_directory_path(m_elem->first);

        Blob file;
        restore_blob_from_full_id(m_elem->second, file);

        ofstream ofs(m_elem->first);
        ofs << file.get_content();
        ofs.close();
    }

    return 0;

}

int vms_merge(const char* given_branch, const char* current_branch) {

    // Ask user confirmation before merging
    string input;
    cout << "Merging branch " << given_branch << " into branch " << current_branch << "...\n\n";

    cout << "Warning: merging will clear the staging area and may overwrite uncommitted changes for files in the working directory\n";
    cout << "If you have uncommitted changes you would like to keep, please commit before proceeding with merge\n\n";
    cout << "Confirm merge (y/n):";
    getline(cin, input);

    while (input != "y" && input != "n") {
        cout << "Please enter y or n:";
        getline(cin, input);
    }

    if (input == "n") {
        cout << "Aborting merge..." << endl;
        return -1;
    }


    string split_id;
    string given_branch_id;
    string current_branch_id;

    find_split_point(string(given_branch), string(current_branch), split_id);

    get_id_from_branch(given_branch, given_branch_id);
    get_id_from_branch(current_branch, current_branch_id);

    if (split_id == given_branch_id) { // Given branch is a direct ancestor of current branch, so do nothing
        cout << "\nNot necessary to merge. Given branch is direct ancestor of current branch" << endl;
        return 0;
    }

    string current_branch_fpath;
    if (get_branch_path(current_branch_fpath) != 0) {
        return -1;
    }

    // Otherwise, perform merge

    // Load all commits:
    Commit given_commit;
    Commit current_commit;
    Commit split_commit;

    restore_commit_from_shortened_id(given_branch_id.c_str(), given_commit);
    restore_commit_from_shortened_id(current_branch_id.c_str(), current_commit);
    restore_commit_from_shortened_id(split_id.c_str(), split_commit);

    map<string, string> given_map = given_commit.get_map();
    map<string, string> current_map = current_commit.get_map();
    map<string, string> split_map = split_commit.get_map();

    map<string, string>::iterator map_it;

    stringstream updated_files;
    updated_files << "Updated files\n";

    if (split_id == current_branch_id) { // Current branch is a direct ancestor of given branch, so fast forward merge
        cout << "\nCurrent branch " << current_branch << " is a direct ancestor of given branch " << given_branch << "\n";
        cout << "Fast-forward merging branch " << current_branch <<  " into branch " << given_branch << endl;
        
        // Update files in current working directory with versions in given commit if modified or new, relative to current commit's version.

        for (map_it = given_map.begin(); map_it != given_map.end(); map_it++) {

            RelativeFileStatus rfs = find_relative_file_status(map_it->first, given_map, current_map);
            
            if (rfs == NEW || rfs == MODIFIED) {
                create_directory_path(map_it->first);

                Blob file;
                restore_blob_from_full_id(map_it->second, file);

                ofstream ofs(map_it->first);
                ofs << file.get_content();
                ofs.close();

                updated_files << "    " << map_it->first << "\n";
            }
            

        }

        // Update current branch to point to commit pointed to by given branch (fast forward)   
        create_and_write_file(current_branch_fpath.c_str(), given_branch_id.c_str(), 0644);

        // Load index, clear it, and save it back
        map<string, string> index;  
        restore< map<string, string> >(index, ".vms/index");
        index.clear();
        save< map<string, string> >(index, ".vms/index");

        cout << updated_files.rdbuf() << endl;

        return 0;
    }

    // Otherwise, standard merge

    // add all tracked files to a iterable set
    set<string> files_union;
    set<string>::iterator files_it;

    for (map_it = given_map.begin(); map_it != given_map.end(); map_it++) {
        files_union.insert(map_it->first);
    }

    for (map_it = current_map.begin(); map_it != current_map.end(); map_it++) {
        files_union.insert(map_it->first);
    }

    for (map_it = split_map.begin(); map_it != split_map.end(); map_it++) {
        files_union.insert(map_it->first);
    }

    // load index and clear it
    map<string, string> index;  
    restore< map<string, string> >(index, ".vms/index");
    index.clear();

    RelativeFileStatus given_status;
    RelativeFileStatus current_status;

    for (files_it = files_union.begin(); files_it != files_union.end(); files_it++) {
        given_status = find_relative_file_status(*files_it, given_map, split_map);
        current_status = find_relative_file_status(*files_it, current_map, split_map);

        if ( (given_status == NEW && current_status == NOT_FOUND) || 
             (given_status == MODIFIED && current_status == DELETED) || 
             (given_status == MODIFIED && current_status == UNMODIFIED) ) {
            
            // Case 1: Check out file into current directory and add file to staging area (for commit at end)
            map_it = given_map.find(*files_it);
            create_directory_path(map_it->first);

            Blob file;
            restore_blob_from_full_id(map_it->second, file);

            ofstream ofs(map_it->first);
            ofs << file.get_content();
            ofs.close();

            index[map_it->first] = map_it->second;

            updated_files << "    " << map_it-> first << "\n";

        } else if ( (current_status == NEW && given_status == NOT_FOUND) ||
                    (current_status == MODIFIED && given_status == DELETED) ||
                    (current_status == MODIFIED && given_status == UNMODIFIED) ) {

            // Case 2: Do nothing: no need to check out or update because already in current directory and tracking

        } else {  // Case 3

            if (given_status == DELETED && current_status == UNMODIFIED) {
                // Change in given branch and no change in current branch
                // Stage file to be deleted in current branch
                index[*files_it] = STAGE_DELETE;

            } else if (current_status == DELETED && given_status == UNMODIFIED) {
                // Change in current branch and no change in given branch
                // Do nothing

            } else if (given_status == DELETED && current_status == DELETED) {
                // Both deleted
                // Do nothing

            } else if (given_status == UNMODIFIED && current_status == UNMODIFIED) {
                // Both unmodified
                // Do nothing

            } else {  // otherwise, exist in both: either modified/modified or new/new: check equality and merge if not equal
                // Check equality: if equal, then do nothing. Move on to next file. If not, then merge conflict.
                map_it = given_map.find(*files_it);
                string given_ver_id = map_it->second;

                map_it = current_map.find(*files_it);
                string current_ver_id = map_it->second;

                if (given_ver_id != current_ver_id) {
                    cout << "\nMerge conflict for file " << *files_it << ": please resolve and commit resolved changes" << endl;
                    // Overwrite file in current directory with formatted and add file to staging area (for commit at end)

                    create_directory_path(*files_it);

                    Blob given_ver_file;
                    restore_blob_from_full_id(given_ver_id, given_ver_file);

                    Blob current_ver_file;
                    restore_blob_from_full_id(current_ver_id, current_ver_file);

                    ofstream ofs(*files_it);
                    ofs << "<<<<<<< version: " << current_branch << "\n";
                    ofs << current_ver_file.get_content() << "\n";
                    ofs << "=======\n";
                    ofs << given_ver_file.get_content() << "\n";
                    ofs << ">>>>>>> version: " << given_branch << endl;

                    ofs.close();

                    // blob the new file contents and save it.
                    ifstream ifs(*files_it);
                    if (!ifs.is_open()) {
                        cerr << "Error occurred: unable to open file " << *files_it << " for staging" << endl;
                        return -1;
                    }
                    Blob merged_file(ifs);

                    string merged_file_id = merged_file.hash();

                    // Puts filepath and hash into the index map and save the updated index
                    index[*files_it] = merged_file_id;

                    string merged_file_id_prefix;
                    string merged_file_id_suffix;

                    split_prefix_suffix(merged_file_id, merged_file_id_prefix, merged_file_id_suffix, PREFIX_LENGTH);
                    // Save blob in objects
                    ostringstream obj_path;
                    obj_path << ".vms/objects/" << merged_file_id_prefix;
                    mkdir(obj_path.str().c_str(), 0755);
                    obj_path << "/" << merged_file_id_suffix;
                    save<Blob>(merged_file, obj_path.str());

                    updated_files << "    " << map_it->first << "\n";

                }
            }

        }
    }


    // Create new child commit using current branch as first parent and template
    ostringstream message;
    message << "Merge branch " << given_branch << " into branch " << current_branch;
    Commit child_commit(message.str());
    child_commit.set_second_parent(given_branch_id);

    // Update with index.
    for (map_it=index.begin(); map_it!=index.end(); map_it++) {
        // if staged is file mapped to STAGE_DELETE string, remove it from commit tree
        if (map_it->second == STAGE_DELETE) {
            child_commit.remove_from_map(map_it->first);
        } else { // puts entry to commit map
            child_commit.put_to_map(map_it->first, map_it->second);
        }
    }

    // Clear index and save it back;
    index.clear();
    save< map<string, string> >(index, ".vms/index");

    // Update commit pointed to by current branch
    string child_commit_id = child_commit.hash();
    create_and_write_file(current_branch_fpath.c_str(), child_commit_id.c_str(), 0644);

    //Push formatted commit string to log and save
    stack<string> log;
    restore< stack<string> >(log, ".vms/log");

    log.push(child_commit.log_string());
    save< stack<string> >(log, ".vms/log");

    // Serialize and save your commit.
    ostringstream obj_path;

    string child_commit_id_prefix;
    string child_commit_id_suffix;
    split_prefix_suffix(child_commit_id, child_commit_id_prefix, child_commit_id_suffix, PREFIX_LENGTH);

    obj_path << ".vms/objects/" << child_commit_id_prefix;
    make_dir(obj_path.str().c_str());
    obj_path << "/" << child_commit_id_suffix;

    save<Commit>(child_commit, obj_path.str());

    // Change permissions of generated file.
    int ret = chmod(obj_path.str().c_str(), 0444);
    if (ret != 0) {
        cerr << "Error occurred: failed to change permissions of commit file." << obj_path.str() << endl;
        return -1;
    }

    cout << updated_files.rdbuf() << endl;

    return 0;
}