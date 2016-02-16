.686
.MODEL FLAT, C
.STACK

.DATA
    
.CODE

GetThreadTIB PROC
	assume fs:nothing 
	MOV EAX, FS:[0]
	ret
GetThreadTIB ENDP

GetThreadTEB PROC
	assume fs:nothing 
	MOV EAX, FS:[018h]
	ret
GetThreadTEB ENDP

GetThreadPEB PROC
	assume fs:nothing 
	MOV EAX, FS:[030h]
	ret
GetThreadPEB ENDP

END
