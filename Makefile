SOURCE_DIR=src
HEADER_DIR=include
BUILD_DIR=out

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

ALL_SOURCES=$(call rwildcard,$(SOURCE_DIR),*.c)
ALL_OBJECTS=$(patsubst $(SOURCE_DIR)/%.c,$(BUILD_DIR)/%.o,$(ALL_SOURCES))

SOURCES=$(filter-out $(SOURCE_DIR)/test%.c,$(ALL_SOURCES))
OBJECTS=$(patsubst $(SOURCE_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

TARGET=libmdr.a
TEST_TARGET=tests

CFLAGS+=-g -Wall

all: $(TARGET)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	rm -f $(TEST_TARGET)

test: $(TEST_TARGET)

run_test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TARGET): $(OBJECTS)
	$(AR) rcs $@ $^

$(TEST_TARGET): $(ALL_OBJECTS)
	gcc $(CFLAGS) -o $@ $^ -lbluetooth

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c $(HEADER_DIR)/mdr/%.h
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -I $(HEADER_DIR) -o $@ $<

$(BUILD_DIR)/test%.o: $(SOURCE_DIR)/test%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -I $(HEADER_DIR) -o $@ $<

