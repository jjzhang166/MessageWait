MAKE := make --no-print-directory

all :
	@$(MAKE) -f makefile_debug clean;
	@$(MAKE) -f makefile_debug;
	@$(MAKE) -f makefile_release clean;
	@$(MAKE) -f makefile_release;

	@$(MAKE) -f makefile_debug clean;
	@$(MAKE) -f makefile_release clean;
clean:
	@$(MAKE) -f makefile_debug clean;
	@$(MAKE) -f makefile_release clean;
