#include <bits/stdc++.h>
#include <dirent.h>
using namespace std;

// Modify with the data text file
#define data_folder "initial_data/"
// Modify with the instructions text file
#define instruction_file "instructions_input.txt"
// Modify with the writing text file
#define write_folder "final_data/"

int all_deleted_lines = 0;
int all_moved_lines = 0;

ifstream in(instruction_file);

typedef struct my_data my_data;
struct my_data {
    string name;
    int nr_childs;
    int nr_lines;
    vector<my_data> childs;
    vector<string> lines;
    vector<bool> deleted_lines;
    vector<my_data*> file_to_compare;
};

void read_data(my_data &current) {
    in >> current.name;
    string line;
    ifstream read(data_folder + current.name);
    current.nr_lines = 0;
    while (getline(read, line)) {
        current.lines.push_back(line);
        current.deleted_lines.push_back(false);
        ++current.nr_lines;
    }
    in >> current.nr_childs;
    for (int i = 1; i <= current.nr_childs; ++i) {
        my_data new_child;
        read_data(new_child);
        current.childs.push_back(new_child);
    }
}

void write_partial_instructions() {
    ofstream out(instruction_file);
    DIR *directory;
    directory = opendir(data_folder);
    struct dirent *entry;
    while ((entry = readdir(directory))) {
        if (entry->d_name[0] != '.') {
            out << entry->d_name << '\n';
        }
    }
    closedir(directory);
}

void remove_common_lines(my_data &current) {
    for (auto &child : current.childs) {
        remove_common_lines(child);
        current.file_to_compare.push_back(&child);
        for (auto &file : child.file_to_compare) {
            current.file_to_compare.push_back(file);
        }
    }
    for (auto file : current.file_to_compare) {
        for (int i = 0; i < current.nr_lines; ++i) {
            for (int j = 0; j < file->nr_lines; ++j) {
                if (current.lines[i] == "" || file->lines[j] == "") {
                    continue;
                }

                if (current.lines[i][0] == '#' || file->lines[j][0] == '#') {
                    continue;
                }

                if (file->deleted_lines[j] == true) {
                    continue;
                }
                if (current.lines[i] == file->lines[j]) {
                    file->deleted_lines[j] = true;
                }
            }
        }
    }
}

void move_common_lines_to_upper_parents(my_data &current) {
    for (auto &child : current.childs) {
        move_common_lines_to_upper_parents(child);
    }
    if (!current.nr_childs) {
        return;
    }
    vector<string> lines_to_check;
    vector<bool> lines_to_go_up;
    for (auto line : current.childs[0].lines) {
        lines_to_check.push_back(line);
        lines_to_go_up.push_back(true);
    }
    
    for (int i = 0; i < (int) lines_to_check.size(); ++i) {
        if (lines_to_check[i] == "") {
            int j = i - 1;
            while (j >= 0) {
                if (lines_to_check[j] == "") {
                    lines_to_go_up[j] = false;
                    --j;
                    continue;
                }
                if (lines_to_check[j][0] != '#' && lines_to_go_up[j]) {
                    j = -1;
                    continue;
                }
                if (lines_to_check[j][0] == '#') {
                    while (j >= 0 && lines_to_check[j][0] == '#') {
                        lines_to_go_up[j] = false;
                        --j;
                    }
                    j = -1;
                }
                --j;
            }

            continue;
        }
        if (lines_to_check[i][0] == '#') {
            continue;
        }
        bool always_common = true;
        for (auto &child : current.childs) {
            bool common_here = false;
            for (auto line : child.lines) {
                if (line == lines_to_check[i]) {
                    common_here = true;
                    break;
                }
            }
            if (!common_here) {
                always_common = false;
                break;
            }
        }
        if (!always_common) {
            lines_to_go_up[i] = false;
        }
        else {
            for (auto &child : current.childs) {
                int j = 0;
                for (auto line : child.lines) {
                    if (line == lines_to_check[i]) {
                        child.deleted_lines[j] = true;
                        break;
                    }
                    ++j;
                }
            }
        }
    }

    int current_insert_position = 0;
    for (int i = 0; i < (int) lines_to_check.size(); ++i) {
        if (lines_to_go_up[i]) {
            auto it = current.lines.begin();
            current.lines.insert(it + current_insert_position, lines_to_check[i]);
            ++current.nr_lines;
            auto it1 = current.deleted_lines.begin();
            current.deleted_lines.insert(it1 + current_insert_position, false);
            ++current_insert_position;
            ++all_moved_lines;
        }
    }
}

void remove_unnecessary_comments(my_data &current) {
     
    int n = current.lines.size();
    for (int i = 0; i < n; ++i) {
        string line = current.lines[i];
        if (line[0] == '#' && !current.deleted_lines[i]) {
            int j = i;
            int end = i;
            bool comment_to_delete = true;
            while(j < n) {
                line = current.lines[j];
                if (line == "") {
                    end = j;
                    j = n + 1;
                    continue;
                }
                if (line[0] == '#') {
                    end = j;
                }
                if (line[0] != '#' && !current.deleted_lines[j]) {
                    comment_to_delete = false;
                    j = n + 1;
                }
                ++j;
            }
            if (comment_to_delete) {
                for (int j = i; j <= end; ++j) {
                    current.deleted_lines[j] = true;
                }
            }
        }
    }

    for (auto &child : current.childs) {
        remove_unnecessary_comments(child);
    }
}

void  write_refractored_files(my_data &current) {
    ofstream write(write_folder + current.name);
    int i = 0;
    for (auto line : current.lines) {
        if (!current.deleted_lines[i]) {
            write << line << '\n';
        }
        else {
            ++all_deleted_lines;
        }
        ++i;
    }
    for (auto &child : current.childs) {
        write_refractored_files(child);
    }
}

int main() {

    my_data root;
    // Run this only if it's the first time making the instructions
    // write_partial_instructions();
    read_data(root);
    remove_common_lines(root);
    move_common_lines_to_upper_parents(root);
    remove_unnecessary_comments(root);
    write_refractored_files(root);

    cout << "Number of deleted lines after refractoring : " << all_deleted_lines << '\n';
    cout << "Number of moved lines after refractoring : "<< all_moved_lines << '\n';

    return 0;
}