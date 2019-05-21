CC:=gcc
CFLAGS:= -Wall -Werror -pthread -o
TARGETS:= app
OBJS = test.o mapreduce.o
OBJ1 = test.o
OBJ2 = mapreduce.o
SRC1 = wordcount.c
SRC2 = mapreduce.c
TEST1 = testinput/input1.txt
TEST2 = testinput/input2.txt
TEST3 = testinput/input3.txt
TEST4 = testinput/input4.txt
TEST5 = testinput/input5.txt

all: $(OBJS)
	$(CC) $(CFLAGS) $(TARGETS) $(OBJS)
test.o: $(SRC1)
	$(CC) $(CFLAGS) $(OBJ1) -c $(SRC1)
mapreduce.o: $(SRC2)
	$(CC) $(CFLAGS) $(OBJ2) -c $(SRC2)
test-p4b: $(TARGETS)
	@echo "\nTest1: Three 1s\n"
	./$(TARGETS) $(TEST1)
	@echo "\nTest2: 14 cats and 4 dogs\n"
	./$(TARGETS) $(TEST2)
	@echo "\nTest3: One single sentence\n"
	./$(TARGETS) $(TEST3)
	@echo "\nTest4: Paragraph from Old Man and the Sea\n"
	./$(TARGETS) $(TEST4)
	@echo "\nTest5: Non-existent file\n"
	./$(TARGETS) $(TEST5)

handin: handin-check
	@echo "Handing in with git (this may ask for your GitHub username/password)..."
	@git push origin master


handin-check:
	@if ! test -d .git; then \
		echo No .git directory, is this a git repository?; \
		false; \
	fi
	@if test "$$(git symbolic-ref HEAD)" != refs/heads/master; then \
		git branch; \
		read -p "You are not on the master branch.  Hand-in the current branch? [y/N] " r; \
		test "$$r" = y; \
	fi
	@if ! test -e info.txt; then \
		echo "You haven't created an info.txt file!"; \
		echo "Please create one with your name, email, etc."; \
		echo "then add and commit it to continue."; \
		false; \
	fi
	@if ! git diff-files --quiet || ! git diff-index --quiet --cached HEAD; then \
		git status -s; \
		echo; \
		echo "You have uncomitted changes.  Please commit or stash them."; \
		false; \
	fi
	@if test -n "`git status -s`"; then \
		git status -s; \
		read -p "Untracked files will not be handed in.  Continue? [y/N] " r; \
		test "$$r" = y; \
	fi

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(TARGETS) *.o


.PHONY: clean handin-check
