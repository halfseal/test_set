#!/bin/bash

# Output file path
FILE="random_data_with_duplicates.bin"

# Remove existing file if it exists
if [ -f "$FILE" ]; then
    rm "$FILE"
    echo "Existing file removed: $FILE"
fi

# Temporary directory for individual pages
TMP_DIR="tmp_pages"
mkdir -p "$TMP_DIR"

TOTAL_PAGES=1000000  # Total number of pages
DUPLICATE_PAGES=100000  # Pages that will be duplicated
MAX_DUPLICATES=255  # Max repetitions for a page

# Counter to keep track of total written pages
total_written_pages=0

# Step 1: Generate 10,000 unique pages (4KB each)
for i in $(seq 1 $DUPLICATE_PAGES); do
    dd if=/dev/urandom bs=4K count=1 of="$TMP_DIR/page_$i.bin" status=none
done

# Step 3: Write remaining unique pages if needed
dd if=/dev/urandom bs=4K count=900000 of="$FILE" status=none
ls -lh random_data_with_duplicates.bin

# Step 2: Write 10,000 duplicate pages with controlled repetition
for i in $(seq 1 $DUPLICATE_PAGES); do
    REPEAT=$((RANDOM % MAX_DUPLICATES + 2))  # Random repetition (2 to 255)

    for j in $(seq 1 $REPEAT); do
        cat "$TMP_DIR/page_$i.bin" >> "$FILE"
        total_written_pages=$((total_written_pages + 1))
	echo $total_written_pages

        # Break if total pages reach 10,000
        if [ "$total_written_pages" -ge "$DUPLICATE_PAGES" ]; then
            echo "Reached the limit of 10,000 pages."
            ls -lh random_data_with_duplicates.bin
            break 2  # Exit both loops
        fi
    done
done



# Clean up temporary files
rm -r "$TMP_DIR"

echo "Random data file with duplicates generated: $FILE"
echo "Total file size: $(du -h $FILE | cut -f1)"

