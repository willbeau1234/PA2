#include "mgit.h"
#include <errno.h>
#include <zstd.h>

// --- Helper Functions ---
uint32_t get_current_head()
{
    // TODO: Read the integer from ".mgit/HEAD" and return it. Return 0 if it fails.
    FILE *f = fopen(".mgit/HEAD", "r");
    if (!f) {
        return 0;
    }
    
    uint32_t id = 0;
    if (fscanf(f, "%u", &id) != 1) {
        fclose(f);
        return 0;
    }

    fclose(f);
    return id;
}

void update_head(uint32_t new_id)
{
    // TODO: Overwrite ".mgit/HEAD" with the new_id.
    FILE *f = fopen(".mgit/HEAD", "w");
    if (!f) {
        perror("fopen .mgit/HEAD");
        exit(1);
    }

    fprintf(f, "%u", new_id);
    fclose(f);
}

// --- Blob Storage (Raw) ---
void write_blob_to_vault(const char* filepath, BlockTable* block)
{
    // TODO: Open `filepath` for reading (rb).
    FILE *in = fopen(filepath, "rb");
    if (!in) {
        perror("fopen input file");
        exit(1);
    }

    // TODO: Open `.mgit/data.bin` for APPENDING (ab).
    FILE *vault = fopen(".mgit/data.bin", "ab");
    if (!vault) {
        perror("fopen .mgit/data.bin");
        fclose(in);
        exit(1);
    }

    if (fseek(vault, 0, SEEK_END) != 0) {
        perror("fseek vault");
        fclose(in);
        fclose(vault);
        exit(1);
    }

    // TODO: Use ftell() to record the current end of the vault into block->physical_offset.
    long pos = ftell(vault);
    if (pos < 0) {
        perror("ftell vault");
        fclose(in);
        fclose(vault);
        exit(1);
    }

    block -> physical_offset = (uint64_t)pos;
    block -> compressed_size = 0;
    char buf[8192];
    size_t i;

    // TODO: Read the file bytes and write them into the vault. Update block->size.
    while ((i = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, i, vault) != i) {
            perror("fwrite vault:)");
            fclose(in);
            fclose(vault);
            exit(1);
        }

        block -> size += (uint32_t)i;
    }

    fclose(in);
    fclose(vault);
}

void read_blob_from_vault(uint64_t offset, uint32_t size, int out_fd)
{
    // TODO: Open the vault, fseek() to the physical_offset.
    // TODO: Read `size` bytes and write them to `out_fd` using the write_all() helper.
}

// --- Snapshot Management ---
void store_snapshot_to_disk(Snapshot* snap)
{
    // TODO: Serialize the Snapshot struct and its linked list of FileEntry/BlockTables
    // into a binary file inside `.mgit/snapshots/snap_XXX.bin`.
    char path[128];
    snprintf(path, sizeof(path), ".mgit/snapshots/snap_%03u.bin", snap -> snapshot_id);

    FILE *f = fopen(path, "wb");
    if (!f) {
        perror("fopen snapshot");
        exit(1);
    }

    if(fwrite(&snap -> snapshot_id, sizeof(uint32_t), 1, f) != 1 ||
       fwrite(&snap -> file_count, sizeof(uint32_t), 1, f) != 1 ||
       fwrite(snap -> message, sizeof(snap -> message), 1, f) != 1) {
        perror("fwrite snapshot header");
        fclose(f);
        exit(1);
       }

    FileEntry *curr = snap -> files;
    while (curr) {
        if (fwrite(curr -> path, sizeof(curr -> path), 1, f) != 1 ||
            fwrite(&curr -> size, sizeof(off_t), 1, f) != 1 ||
            fwrite(curr -> checksum, sizeof(curr -> checksum), 1, f) != 1 ||
            fwrite(&curr -> is_directory, sizeof(int), 1, f) != 1 ||
            fwrite(&curr -> num_blocks, sizeof(int), 1, f) != 1 ||
            fwrite(&curr -> mtime, sizeof(time_t), 1, f) != 1 || 
            fwrite(&curr -> inode, sizeof(ino_t), 1, f) != 1) {
                perror("fwrite file entry");
                fclose(f);
                exit(1);
        }

        if (curr -> num_blocks > 0 && curr -> chunks) {
            if (fwrite(curr -> chunks, sizeof(BlockTable), curr -> num_blocks, f) != (size_t) curr -> num_blocks) {
                perror("fwrite block table");
                fclose(f);
                exit(1);
            }
        }

        curr = curr -> next;
    }

    fclose(f);
}

Snapshot* load_snapshot_from_disk(uint32_t id)
{
    // TODO: Read a `snap_XXX.bin` file and reconstruct the Snapshot struct
    // and its FileEntry linked list in heap memory.
    return NULL;
}

void chunks_recycle(uint32_t target_id)
{
    // TODO: Garbage Collection (The Vacuum)
    // 1. Load the oldest snapshot (target_id) and the newest snapshot (HEAD).
    // 2. Iterate through the oldest snapshot's files.
    // 3. If a chunk's physical_offset is NOT being used by ANY file in the HEAD snapshot,
    //    it is "stalled". Zero out those specific bytes in `data.bin`.
}

void mgit_snapshot(const char* msg)
{
    // TODO: 1. Get current HEAD ID and calculate next_id. Load previous files for crawling.
    // TODO: 2. Call build_file_list_bfs() to get the new directory state.

    // TODO: 3. Iterate through the new file list.
    // - If a file has data (chunks) but its size is 0, it needs to be written to the vault.
    // - CRITICAL: Check for Hard Links! If another file in the *current* list with the same
    //   inode was already written to the vault, copy its offset and size. DO NOT write twice!
    // - Call write_blob_to_vault() for new files.

    // TODO: 4. Call store_snapshot_to_disk() and update_head().
    // TODO: 5. Free memory.
    // TODO: 6. Enforce MAX_SNAPSHOT_HISTORY (5). If exceeded, call chunks_recycle()
    //          and delete the oldest manifest file using remove().
}
