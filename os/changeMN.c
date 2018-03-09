#include <stdio.h>
#include <stdlib.h>

int main() {
  int ret;
  FILE *fp_read;
  FILE *fp_write;
  unsigned char buf[2048];
  
  fp_read = fopen("./myfs", "rb");
/*Change to it when testing at section 2.4*/
  /*fp_read = fopen("./tmpfs", "rb");*/ 
	
  if(fp_read == NULL) {
    fprintf(stderr, "open myfs failed!\n");
    return EXIT_FAILURE;
  }

  fp_write = fopen("./fs.new", "wb");
  
  if(fp_write == NULL) {
    fprintf(stderr, "open fs.new failed!\n");
    return EXIT_FAILURE;
  }
  
  ret = fread(buf, sizeof(unsigned char), 2048, fp_read);

  printf("previous magic number is 0x%x%x\n", buf[0x438], buf[0x439]);

  buf[0x438] = 0x66;
  buf[0x439] = 0x66;

  fwrite(buf, sizeof(unsigned char), 2048, fp_write);

  printf("current magic number is 0x%x%x\n", buf[0x438], buf[0x439]);

  while(ret == 2048) {
    ret = fread(buf, sizeof(unsigned char), 2048, fp_read);
    fwrite(buf, sizeof(unsigned char), ret, fp_write);
  }

  if(ret < 2048 && feof(fp_read)) {
    printf("change magic number ok!\n");
  }

  fclose(fp_read);
  fclose(fp_write);

  return EXIT_SUCCESS;
}

