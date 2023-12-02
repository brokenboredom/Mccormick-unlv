# invoke SourceDir generated makefile for helloHWI.pem4f
helloHWI.pem4f: .libraries,helloHWI.pem4f
.libraries,helloHWI.pem4f: package/cfg/helloHWI_pem4f.xdl
	$(MAKE) -f C:\Users\samue\ccs_workspace_v12\helloHWI_MSP_EXP432E401Y_tirtos_ccs/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\samue\ccs_workspace_v12\helloHWI_MSP_EXP432E401Y_tirtos_ccs/src/makefile.libs clean

