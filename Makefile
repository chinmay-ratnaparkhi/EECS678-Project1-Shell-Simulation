all:
	gcc main.c -o quash
	@echo 
	@echo ‘****************************************’
	@echo Done! Please execute by typing ./quash
	@echo ‘****************************************’
	@echo 
	@echo
	
clean:
	rm  *.o *.out quash


