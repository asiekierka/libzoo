static %TOKEN_TYPE% %STEM%_search(const %STRUCT_TYPE% *table, const char *tok) {
	// TODO: optimize this
	while (table->word[0] != '\0') {
		if (table->word[0] == tok[0]) {
			if (!strcmp(tok + 1, table->word + 1)) {
				return table->id;
			}
		}
		table++;
	}
	return 255;
}
