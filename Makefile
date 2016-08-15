.PHONY: clean All

All:
	@echo "----------Building project:[ buddhabrot - Debug ]----------"
	@$(MAKE) -f  "buddhabrot.mk"
clean:
	@echo "----------Cleaning project:[ buddhabrot - Debug ]----------"
	@$(MAKE) -f  "buddhabrot.mk" clean
