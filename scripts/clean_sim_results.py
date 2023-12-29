import os
import fnmatch

# Set the path to the main folder where the subfolders are located
main_folder = '../data'
# 
work_dir = '../data/work'

# Function to delete the specified files in a directory
def delete_files_in_directory(directory):
    for filename in os.listdir(directory):
        file_path = os.path.join(directory, filename)
        if os.path.isfile(file_path) and (
            filename.startswith('log_') or 
            filename.endswith('.log') or 
            filename.startswith('output_c') or
            filename.startswith('output_test_')
        ):
            print(f"Deleting file: {file_path}")
            os.remove(file_path)

def delete_matlab_files_in_directory(directory):
    for filename in os.listdir(directory):
        file_path = os.path.join(directory, filename)
        if os.path.isfile(file_path) and (
            filename.startswith('input') or  
            filename.startswith('output_ref')
        ):
            print(f"Deleting file: {file_path}")
            os.remove(file_path)


# Walking through the main folder and processing the subdirectories
delete_matlab_files = input("Delete MATLAB files? (y/n): ")
for dirpath, dirnames, files in os.walk(main_folder):
    for dirname in fnmatch.filter(dirnames, 'testcase*'):
        full_dir_path = os.path.join(dirpath, dirname)
        print(f"Processing directory: {full_dir_path}")
        delete_files_in_directory(full_dir_path)
        if delete_matlab_files == 'y':
            delete_matlab_files_in_directory(full_dir_path)

# Delete files in the work directory
if os.path.exists(work_dir):
    # Iterate over all the files in the specified directory
    for filename in os.listdir(work_dir):
        file_path = os.path.join(work_dir, filename)
        
        # Check if it's a file and not a subdirectory
        if os.path.isfile(file_path):
            try:
                os.remove(file_path)
                print(f"Deleted file: '{file_path}'")
            except Exception as e:
                print(f"Failed to delete file '{file_path}'. Reason: {e}")
else:
    print(f"The directory {work_dir} does not exist.")
