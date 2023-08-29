TEST_SCRIPTS = $(wildcard *.sh)
BLAZE = $(realpath ../src/blaze)

all:
	@export BLAZE="$(BLAZE)"; \
	export FILE="$$(pwd)/tmp.bl"; \
	for test in $(TEST_SCRIPTS); do \
		if test "$$test" = "setup.sh"; then \
			continue; \
		fi; \
		printf "\033[1;36mTEST\033[0m \033[1m%s\033[0m\n" $$test; \
		sh $$test; \
		exitcode=$$?; \
		rm -f $$FILE; \
		if test "$$exitcode" = "0"; then \
			printf "\033[1;32mPASS\033[0m \033[1m%s\033[0m\n" $$test; \
		else \
			printf "\033[1;31mFAIL\033[0m \033[1m%s\033[0m\n" $$test; \
			printf "Error code: %d\n" $$exitcode; \
			exit 1; \
		fi; \
		echo ""; \
	done
