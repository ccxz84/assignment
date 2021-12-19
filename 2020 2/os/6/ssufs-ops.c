#include "ssufs-ops.h"

extern struct filehandle_t file_handle_array[MAX_OPEN_FILES];

int ssufs_allocFileHandle() {
	for(int i = 0; i < MAX_OPEN_FILES; i++) {
		if (file_handle_array[i].inode_number == -1) {
			return i;
		}
	}
	return -1;
}

int ssufs_create(char *filename){
	/* 1 */
	int node, block;
	if((node = ssufs_allocInode()) == -1){
		return -1;
	}

	if((block = ssufs_allocDataBlock()) == -1){
		return -1;
	}
	ssufs_writeDataBlock(block, "");
	struct inode_t *inode = malloc(sizeof(struct inode_t));
	ssufs_readInode(node, inode);
	inode->status = INODE_IN_USE;
	strcpy(inode->name, filename);
	inode->direct_blocks[0] = block;
	inode->file_size = 0;
	ssufs_writeInode(node, inode);
}

void ssufs_delete(char *filename){
	/* 2 */
	int inode;

	if((inode = open_namei(filename)) == -1){
		return;
	}

	for(int i = 0; i < MAX_OPEN_FILES; i++) {
		if (file_handle_array[i].inode_number == inode) {
			ssufs_close(i);
		}
	}

	ssufs_freeInode(inode);
}

int ssufs_open(char *filename){
	/* 3 */
	int handle, inode;

	if((inode = open_namei(filename)) == -1){
		return -1;
	}
	if((handle = ssufs_allocFileHandle()) == -1){
		return -1;
	}

	file_handle_array[handle].inode_number = inode;
	file_handle_array[handle].offset = 0;
	return handle;
}

void ssufs_close(int file_handle){
	file_handle_array[file_handle].inode_number = -1;
	file_handle_array[file_handle].offset = 0;
}

int ssufs_read(int file_handle, char *buf, int nbytes){
	/* 4 */
	char blockbuf[BLOCKSIZE], *blockptr, *ptr;
	int blocknum, off,bytes = nbytes,num;
	struct inode_t * inodeptr = malloc(sizeof(struct inode_t));
	ssufs_readInode(file_handle_array[file_handle].inode_number,inodeptr);
	
	if(file_handle_array[file_handle].inode_number == -1)
		return -1;

	if(file_handle_array[file_handle].offset+nbytes > inodeptr->file_size){
		free(inodeptr);
		return -1;
	}

	for(;bytes > 0;){
		blocknum = inodeptr->direct_blocks[file_handle_array[file_handle].offset/BLOCKSIZE];
		off = file_handle_array[file_handle].offset - (BLOCKSIZE*(file_handle_array[file_handle].offset/BLOCKSIZE));
		ssufs_readDataBlock(blocknum,blockbuf);
		blockptr = blockbuf + off;
		num = BLOCKSIZE - off > 0 ? BLOCKSIZE - off : bytes;
		memcpy(buf, blockptr, num);
		bytes -= num;
		file_handle_array[file_handle].offset += num;
	}
	free(inodeptr);
	return 0;
}

int ssufs_write(int file_handle, char *buf, int nbytes){
	/* 5 */
	char blockbuf[BLOCKSIZE], *blockptr, *ptr, temp[BLOCKSIZE];
	int blocknum, off,bytes = nbytes, i, size, prevoffset;
	struct inode_t * inodeptr = malloc(sizeof(struct inode_t)), * prev;
	ssufs_readInode(file_handle_array[file_handle].inode_number,inodeptr);
	
	if(file_handle < 0||file_handle_array[file_handle].inode_number == -1){
		return -1;
	}

	if(file_handle_array[file_handle].offset+nbytes > BLOCKSIZE * MAX_FILE_SIZE){
		free(inodeptr);
		return -1;
	}

	ptr = buf;
	prevoffset = inodeptr->direct_blocks[file_handle_array[file_handle].offset];
	for(;bytes > 0;){
		memset(blockbuf, 0, BLOCKSIZE);
		blocknum = inodeptr->direct_blocks[file_handle_array[file_handle].offset/BLOCKSIZE];
		if(blocknum == -1){
			off = 0;
			if((inodeptr->direct_blocks[file_handle_array[file_handle].offset/BLOCKSIZE] = ssufs_allocDataBlock()) == -1){
				prev = malloc(sizeof(struct inode_t));
				ssufs_readInode(file_handle_array[file_handle].inode_number,prev);
				for(i = prevoffset/BLOCKSIZE + 1 ; i < blocknum; i++){
					if(inodeptr->direct_blocks[i] != -1){
						ssufs_freeDataBlock(inodeptr->direct_blocks[i]);
					}
				}
				if((blocknum = prev->direct_blocks[prevoffset/BLOCKSIZE]) != -1){
					ssufs_readDataBlock(blocknum,blockbuf);
					memset(temp,0,BLOCKSIZE);
					memcpy(temp,blockbuf, prevoffset - (BLOCKSIZE*(prevoffset/BLOCKSIZE)));
					ssufs_writeDataBlock(blocknum, temp);
				}
				return -1;
			}
			blocknum = inodeptr->direct_blocks[file_handle_array[file_handle].offset/BLOCKSIZE];
		}
		else{
			off = file_handle_array[file_handle].offset - (BLOCKSIZE*(file_handle_array[file_handle].offset/BLOCKSIZE));
			ssufs_readDataBlock(blocknum,blockbuf);
		}
		blockptr = blockbuf + off;
		size = bytes - (BLOCKSIZE - off) > 0 ? BLOCKSIZE - off : bytes;	
		memcpy(blockptr, ptr, size);
		file_handle_array[file_handle].offset += size;
		inodeptr->file_size += size;
		ptr += size;
		bytes -= size;
		ssufs_writeDataBlock(blocknum, blockbuf);
	}
	ssufs_writeInode(file_handle_array[file_handle].inode_number,inodeptr);
	free(inodeptr);
	return 0;
}

int ssufs_lseek(int file_handle, int nseek){
	int offset = file_handle_array[file_handle].offset;

	struct inode_t *tmp = (struct inode_t *) malloc(sizeof(struct inode_t));
	ssufs_readInode(file_handle_array[file_handle].inode_number, tmp);
	
	int fsize = tmp->file_size;
	offset += nseek;

	if ((fsize == -1) || (offset < 0) || (offset > fsize)) {

		free(tmp);
		return -1;
	}

	file_handle_array[file_handle].offset = offset;
	free(tmp);

	return 0;
}
