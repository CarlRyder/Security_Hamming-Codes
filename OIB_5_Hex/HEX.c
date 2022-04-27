// Made by Y. Sendov. April 2022

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>
#include <string.h>

#define DEFAULT_ERROR -1
#define BITS_IN_BYTE 8

#define BLOCK_1 8
#define BLOCK_2 12
#define BLOCK_3 16
#define BLOCK_4 24
#define BLOCK_5 32
#define BLOCK_6 48
#define BLOCK_7 64

#define COUNT_CONTROL_BITS_4 4
#define COUNT_CONTROL_BITS_5 5
#define COUNT_CONTROL_BITS_6 6
#define COUNT_CONTROL_BITS_7 7

#define CONTROL_BIT_1 1
#define CONTROL_BIT_2 2
#define CONTROL_BIT_3 4
#define CONTROL_BIT_4 8
#define CONTROL_BIT_5 16
#define CONTROL_BIT_6 32
#define CONTROL_BIT_7 64

int security(char input[3])
{
	int flag = 0;
	if (input[0] == '\n') return 1;
	input[strcspn(input, "\n")] = 0;
	for (unsigned int i = 0; i < strlen(input); i++)
	{
		if (input[i] < 48 || input[i] > 57)
		{
			flag = 1;
			break;
		}
	}
	return flag;
}

// Information about blocks, the total word size and the number of control bits
struct blocks
{
	int block_size; // Block size
	int end_size; // Word size
	int control_bits; // Number of control bits
	int ideal_count;
};

int block_check(int block)
{
	int flag = 0;
	int blocks[7] = { BLOCK_1, BLOCK_2, BLOCK_3, BLOCK_4, BLOCK_5, BLOCK_6, BLOCK_7 };
	for (int i = 0; i < 7; i++) if (block == blocks[i]) flag = 1;
	return flag;
}

void block_rec(int block, struct blocks* info)
{
	int blocks[7] = { BLOCK_1, BLOCK_2, BLOCK_3, BLOCK_4, BLOCK_5, BLOCK_6, BLOCK_7 };
	info->block_size = block;
	if (block == blocks[0])
	{
		info->end_size = blocks[0] + COUNT_CONTROL_BITS_4;
		info->control_bits = COUNT_CONTROL_BITS_4;
	}
	else if (block == blocks[1])
	{
		info->end_size = blocks[1] + COUNT_CONTROL_BITS_4;
		info->control_bits = COUNT_CONTROL_BITS_4;
	}
	else if (block == blocks[2])
	{
		info->end_size = blocks[2] + COUNT_CONTROL_BITS_5;
		info->control_bits = COUNT_CONTROL_BITS_5;
	}
	else if (block == blocks[3])
	{
		info->end_size = blocks[3] + COUNT_CONTROL_BITS_5;
		info->control_bits = COUNT_CONTROL_BITS_5;
	}
	else if (block == blocks[4])
	{
		info->end_size = blocks[4] + COUNT_CONTROL_BITS_6;
		info->control_bits = COUNT_CONTROL_BITS_6;
	}
	else if (block == blocks[5])
	{
		info->end_size = blocks[5] + COUNT_CONTROL_BITS_6;
		info->control_bits = COUNT_CONTROL_BITS_6;
	}
	else if (block == blocks[6])
	{
		info->end_size = blocks[6] + COUNT_CONTROL_BITS_7;
		info->control_bits = COUNT_CONTROL_BITS_7;
	}
}

void size(struct blocks* info, int len)
{
	printf("\nEnter the size of the text block: ");
	int value = -1;
	char input[3];
	fgets(input, 3, stdin);
	input[strcspn(input, "\n")] = 0;
	fseek(stdin, 0, SEEK_END);
	if (security(input) == 0 && block_check(atoi(input)) == 1 && len % atoi(input) == 0)
	{
		int block = atoi(input);
		block_rec(block, info);
	}
	else
	{
		if (len % atoi(input) != 0)
		{
			printf("The size of the block you entered does not fit the encoded text. Try again!\n");
			size(info, len);
		}
		else
		{
			printf("You entered the size of the text block incorrectly. Try again!\n");
			size(info, len);
		}
	}
}

void bits_control(int iter, int byte, unsigned char* binary_control_text)
{
	int control_bits[7] = { CONTROL_BIT_1 - 1, CONTROL_BIT_2 - 1, CONTROL_BIT_3 - 1, CONTROL_BIT_4 - 1, CONTROL_BIT_5 - 1, CONTROL_BIT_6 - 1, CONTROL_BIT_7 - 1 };
	int binary_view[8] = { 0 };
	iter++;
	for (int i = 0; i < BITS_IN_BYTE; i++)
	{
		if (iter == 0) break;
		binary_view[i] = iter % 2;
		if (binary_view[i] != 0)
		{
			binary_control_text[control_bits[i] + byte]++;
		}
		iter /= 2;
	}
}

void coding(struct blocks* info)
{
	FILE* input = fopen("text.txt", "r");
	if (input == NULL)
	{
		printf("File not found! Expected: \"text.txt\"\nCheck the file path and try again.\n");
		exit(DEFAULT_ERROR);
	}
	// Reading and translating text into binary form
	char* binary_text = NULL; // Pointer to bit text
	char* temp; // Temporary pointer
	int binary_len = 0; // The initial length of the bit text
	while (!feof(input))
	{
		char symbol = fgetc(input);
		if (symbol != EOF)
		{
			temp = (char*)realloc(binary_text, binary_len + 8 * sizeof(char));
			if (temp != NULL)
			{
				binary_text = temp;
				for (int i = 0; i < 8; i++)
				{
					binary_text[binary_len + i] = symbol % 2;
					symbol /= 2;
				}
			}
			binary_len += 8;
		}
	}
	info->ideal_count = binary_len;
	fclose(input);
	size(info, binary_len); // Entering the size of a text block processed within a single encoding cycle
	int byte = 0; // Current block (byte)
	int new_binary_len = (binary_len / info->block_size) * info->end_size; // New bit file length
	char* binary_control_text = (char*)malloc(new_binary_len * sizeof(char));
	if (binary_control_text != NULL)
	{
		int k = 0; // Counter for bits
		while (k < binary_len)
		{
			// Inserting control bits into an extended block
			for (int i = 0; i < info->end_size; i++)
			{
				int flag = 0;
				int control_bits[7] = { CONTROL_BIT_1 - 1, CONTROL_BIT_2 - 1, CONTROL_BIT_3 - 1, CONTROL_BIT_4 - 1, CONTROL_BIT_5 - 1, CONTROL_BIT_6 - 1, CONTROL_BIT_7 - 1 };
				for (int j = 0; j < 7; j++)
				{
					if (i == control_bits[j])
					{
						binary_control_text[byte + i] = 0;
						flag = 1;
					}
				}
				if (flag == 0)
				{
					binary_control_text[byte + i] = binary_text[k];
					k++;
				}
			}
			// Counting control bits
			for (int i = 0; i < info->end_size; i++)
			{
				if (binary_control_text[i + byte] != 0)
				{
					bits_control(i, byte, binary_control_text);
				}
			}
			// Checking all groups controlled by the verification bits for the parity of units
			for (int i = 0; i < info->end_size; i++)
			{
				if (binary_control_text[i + byte] % 2 == 0) binary_control_text[i + byte] = 0;
				else binary_control_text[i + byte] = 1;
			}
			byte += info->end_size;
		}
	}
	// Writing encoded text to a file
	FILE* output = fopen("codedtext.txt", "w");
	int counter = 0;
	for (int i = 0; i < byte; i += 8)
	{
		int letter = 0;
		for (int k = 0; k < BITS_IN_BYTE; k++)
		{
			letter = letter << 1;
			letter += binary_control_text[i + 7 - k]; // Writes bits in reverse order
			if (counter == 7)
			{
				unsigned char symbol = (unsigned char)letter;
				fprintf(output, "%c", symbol);
				letter = 0;
				counter = 0;
			}
			else counter++;
		}
		fprintf(output, "\0");
	}
	fclose(output);
	// Clearing memory, zeroing pointers and variables
	free(binary_text);
	binary_text = NULL;
	free(binary_control_text);
	binary_control_text = NULL;
	binary_len = 0;
	new_binary_len = 0;
	byte = 0;
	// Result
	system("cls");
	printf("The text file has been successfully encoded! The encoded text is contained in the file \"codedtext.txt\"\n\n");
}

void bits_control_check(int iter, int bits[])
{
	int control_bits[7] = { CONTROL_BIT_1 - 1, CONTROL_BIT_2 - 1, CONTROL_BIT_3 - 1, CONTROL_BIT_4 - 1, CONTROL_BIT_5 - 1, CONTROL_BIT_6 - 1, CONTROL_BIT_7 - 1 };
	int binary_view[8] = { 0 };
	// If the control bit is all right
	for (int i = 0; i < 7; i++)
	{
		if (iter == control_bits[i]) return;
	}
	iter++;
	for (int i = 0; i < BITS_IN_BYTE; i++)
	{
		binary_view[i] = iter % 2;
		iter /= 2;
	}
	for (int i = 0; i < BITS_IN_BYTE; i++)
	{
		if (binary_view[i] != 0) bits[i] = 1 - bits[i];
	}
}

void decoding(struct blocks* info, int flag)
{
	if (flag == 0)
	{
		system("cls");
		printf("You haven't encoded a text file yet! Try again.\n\n");
		return;
	}
	FILE* input = fopen("codedtext.txt", "r");
	if (input == NULL)
	{
		printf("File not found! Expected: \"codedtext.txt\"\nCheck the file path and try again.\n");
		exit(DEFAULT_ERROR);
	}
	// Reading and translating text into binary form
	unsigned char* binary_coded_text = NULL; // Pointer to bit text
	unsigned char* ptemp; // Temporary pointer
	int binary_len = 0; // The initial length of the bit text
	while (!feof(input))
	{
		char symb = fgetc(input);
		if (symb == EOF) break;
		unsigned char symbol = (unsigned char)symb;
		ptemp = (char*)realloc(binary_coded_text, binary_len + 8 * sizeof(char));
		if (ptemp != NULL)
		{
			binary_coded_text = ptemp;
			for (int i = 0; i < 8; i++)
			{
				binary_coded_text[binary_len + i] = symbol % 2;
				symbol /= 2;
			}
		}
		binary_len += 8;
	}
	fclose(input);
	int byte = 0; // Current block (byte)
	char* decoded_text = (char*)malloc(info->ideal_count * sizeof(char));
	if (decoded_text != NULL && binary_coded_text != NULL)
	{
		while (byte < binary_len)
		{
			int begin_control_bits[7] = { 0 };
			int end_control_bits[7] = { 0 };
			int control_bits[7] = { CONTROL_BIT_1 - 1, CONTROL_BIT_2 - 1, CONTROL_BIT_3 - 1, CONTROL_BIT_4 - 1, CONTROL_BIT_5 - 1, CONTROL_BIT_6 - 1, CONTROL_BIT_7 - 1 };
			// We remember the initial control bits
			for (int i = 0; i < 7; i++)
			{
				if (control_bits[i] < info->end_size)
				{
					begin_control_bits[i] = binary_coded_text[byte + control_bits[i]];
				}
				else begin_control_bits[i] = 0;
			}
			// Getting new control bits
			for (int i = 0; i < info->end_size; i++)
			{
				if (binary_coded_text[i + byte] != 0)
				{
					bits_control_check(i, end_control_bits);
				}
			}
			// We compare the original and new control bits. If they differ, it means that the information was transmitted with an error
			int index_mistake = 0;
			for (int i = 0; i < info->control_bits; i++)
			{
				if (begin_control_bits[i] != end_control_bits[i])
				{
					index_mistake += control_bits[i] + 1;
				}
			}
			index_mistake--; // We reduce the error index by one, since we get an extra one at the end
			// If there is a mistake, we correct it
			if (index_mistake >= 0)
			{
				binary_coded_text[index_mistake + byte] = 1 - binary_coded_text[index_mistake + byte];
			}
			byte += info->end_size;
		}
		// Removing the control bits
		int k = 0; // Counter for bits
		byte = 0;
		while (k < info->ideal_count)
		{
			for (int i = 0; i < info->end_size; i++)
			{
				int flag = 0;
				int control_bits[7] = { CONTROL_BIT_1 - 1, CONTROL_BIT_2 - 1, CONTROL_BIT_3 - 1, CONTROL_BIT_4 - 1, CONTROL_BIT_5 - 1, CONTROL_BIT_6 - 1, CONTROL_BIT_7 - 1 };
				for (int j = 0; j < 7; j++) if (i == control_bits[j]) flag = 1;
				if (flag == 0)
				{
					decoded_text[k] = binary_coded_text[i + byte];
					k++;
				}
			}
			byte += info->end_size;
		}
		int count = 0;
		// Writing to a decoded text file
		FILE* output = fopen("decodedtext.txt", "w");
		while (count < info->ideal_count)
		{
			int word[BITS_IN_BYTE] = { 0 };
			for (int i = 0; i < BITS_IN_BYTE; i++)
			{
				word[i] = decoded_text[i + count];
			}
			unsigned char symbol = 0;
			for (int j = 0; j < BITS_IN_BYTE; j++)
			{
				symbol = symbol << 1;
				symbol += word[7 - j];
			}
			fprintf(output, "%c", symbol);
			count += BITS_IN_BYTE;
		}
		fclose(output);
	}
	// Clearing memory
	free(binary_coded_text);
	binary_coded_text = NULL;
	free(decoded_text);
	decoded_text = NULL;
	// Result
	system("cls");
	printf("The text file has been successfully decoded! The decoded text is contained in the file \"decodedtext.txt\"\n\n");
}

int menu()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	printf("Application menu \"Encoder\"\n\n"
		   "1. Encoding a text file\n"
		   "2. Decoding a text file\n"
		   "3. Exiting the program\n"
	       "\nSelect the number of the program's operating mode: ");
	int value = -1;
	char input[3];
	fgets(input, 3, stdin);
	input[strcspn(input, "\n")] = 0;
	fseek(stdin, 0, SEEK_END);
	if (security(input) == 0 && atoi(input) < 4)
	{
		return atoi(input);
	}
	else return -1;
}

int main()
{
	struct blocks info = { 0, 0, 0, 0 };
	int flag = 0;
	int n = menu();
	while (1)
	{
		switch (n)
		{
		case 1:
			flag = 1;
			coding(&info);
			break;
		case 2:
			decoding(&info, flag);
			break;
		case 3:
			system("cls");
			printf("You have exited the program.\n");
			return 0;
		default:
			system("cls");
			printf("Incorrect input. Try again!\n\n");
			break;
		}
		n = menu();
	}
}