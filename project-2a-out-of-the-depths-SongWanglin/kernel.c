
/* FILL ME IN! */

void bootmain(){
//	__asm__("movb $'O', 0xb8000;"
//		"movb $0x0f, 0xb8001;"
//		"movb $'K', 0xb8002;"
//		"movb $0x0f, 0xb8003;"
//		);
	const char* str = "OK";
	char* video_ptr = (char*)0xb8000;
	for(unsigned int i = 0, j = 0; *(str+j)!='\0'; ){
//	while( *(str+j)!='\0'){
		*(video_ptr + (i++)) = *(str + (j++));
		*(video_ptr + (i++)) = 0x0f;
	}
	while(1){
		__asm__("hlt;");
	}
}
