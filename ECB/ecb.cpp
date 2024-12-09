#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstdint>
#include <cstring>

using namespace std;

// AES constants
const int Nb = 4; // Number of columns (32-bit words) comprising the State
int Nk;           // Number of 32-bit words comprising the Cipher Key
int Nr;           // Number of rounds

// S-box transformation table
const uint8_t sbox[256] = {
    // 0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,  // 0
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,  // 1
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,  // 2
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,  // 3
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,  // 4
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,  // 5
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,  // 6
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,  // 7
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,  // 8
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,  // 9
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,  // A
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,  // B
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,  // C
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,  // D
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,  // E
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}; // F

// Round constant word array
const uint8_t Rcon[255] = {
    0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8,
    0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3,
    0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f,
    0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d,
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab,
    0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d,
    0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25,
    0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01,
    0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d,
    0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa,
    0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a,
    0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02,
    0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a,
    0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef,
    0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94,
    0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04,
    0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f,
    0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5,
    0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33,
    0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb};

// Function prototypes
void KeyExpansion(const uint8_t *key, uint8_t *w);
void Cipher(uint8_t *in, uint8_t *out, const uint8_t *w);
void SubBytes(uint8_t *state);
void ShiftRows(uint8_t *state);
void MixColumns(uint8_t *state);
void AddRoundKey(uint8_t *state, const uint8_t *w, int round);
void PrintState(const char *label, uint8_t *state);
uint8_t xtime(uint8_t x);
uint8_t Multiply(uint8_t x, uint8_t y);

// Helper functions for image processing
bool ReadBMP(const string &filename, vector<uint8_t> &header, vector<uint8_t> &data);
bool WriteBMP(const string &filename, const vector<uint8_t> &header, const vector<uint8_t> &data);

int main()
{
    // AES key (16 bytes for AES-128)
    uint8_t key[16] = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F};

    // Set Nk and Nr based on key length
    Nk = 4; // 128-bit key
    Nr = Nk + 6;

    // Expand the key
    uint8_t w[176]; // Expanded key for AES-128
    KeyExpansion(key, w);

    // Read the image file (e.g., BMP format)
    string inputImage = "blackbuck.bmp";
    string outputImage = "encrypted.bmp";

    vector<uint8_t> header;
    vector<uint8_t> data;

    if (!ReadBMP(inputImage, header, data))
    {
        cerr << "Error reading image file." << endl;
        return 1;
    }

    // Encrypt the image data using AES in ECB mode
    size_t dataSize = data.size();
    vector<uint8_t> encryptedData(dataSize);

    // Process each 16-byte block
    for (size_t i = 0; i < dataSize; i += 16)
    {
        uint8_t in[16] = {0};
        uint8_t out[16] = {0};

        // Copy data to input block (handle last block if it's less than 16 bytes)
        size_t blockSize = min((size_t)16, dataSize - i);
        memcpy(in, &data[i], blockSize);

        // Encrypt the block
        Cipher(in, out, w);

        // Copy the encrypted block to the output data
        memcpy(&encryptedData[i], out, blockSize);

        // Print intermediate states (optional)
        cout << "\nBlock " << (i / 16) << " Encryption Steps:";
        PrintState("Input Block", in);
        // ... (Detailed prints inside the Cipher function)
        PrintState("Encrypted Block", out);
    }

    // Write the encrypted data to a new image file
    if (!WriteBMP(outputImage, header, encryptedData))
    {
        cerr << "Error writing encrypted image file." << endl;
        return 1;
    }

    cout << "\nEncryption complete. Encrypted image saved as " << outputImage << endl;
    cout << "\nObserve the encrypted image to see the patterns retained, demonstrating the weakness of ECB mode." << endl;

    return 0;
}

// Key Expansion
void KeyExpansion(const uint8_t *key, uint8_t *w)
{
    uint8_t temp[4];
    int i = 0;

    // The first Nk words are the original key
    while (i < Nk)
    {
        w[4 * i + 0] = key[4 * i + 0];
        w[4 * i + 1] = key[4 * i + 1];
        w[4 * i + 2] = key[4 * i + 2];
        w[4 * i + 3] = key[4 * i + 3];
        i++;
    }

    // All other words are found from the previous words
    while (i < Nb * (Nr + 1))
    {
        temp[0] = w[4 * (i - 1) + 0];
        temp[1] = w[4 * (i - 1) + 1];
        temp[2] = w[4 * (i - 1) + 2];
        temp[3] = w[4 * (i - 1) + 3];

        if (i % Nk == 0)
        {
            // RotWord
            uint8_t k = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = k;

            // SubWord
            temp[0] = sbox[temp[0]];
            temp[1] = sbox[temp[1]];
            temp[2] = sbox[temp[2]];
            temp[3] = sbox[temp[3]];

            temp[0] = temp[0] ^ Rcon[i / Nk];
        }

        w[4 * i + 0] = w[4 * (i - Nk) + 0] ^ temp[0];
        w[4 * i + 1] = w[4 * (i - Nk) + 1] ^ temp[1];
        w[4 * i + 2] = w[4 * (i - Nk) + 2] ^ temp[2];
        w[4 * i + 3] = w[4 * (i - Nk) + 3] ^ temp[3];
        i++;
    }
}

// Cipher (encryption)
void Cipher(uint8_t *in, uint8_t *out, const uint8_t *w)
{
    uint8_t state[4 * Nb];
    memcpy(state, in, 4 * Nb);

    PrintState("Initial State", state);
    AddRoundKey(state, w, 0);
    PrintState("After AddRoundKey (Round 0)", state);

    for (int round = 1; round < Nr; round++)
    {
        SubBytes(state);
        PrintState("After SubBytes", state);

        ShiftRows(state);
        PrintState("After ShiftRows", state);

        MixColumns(state);
        PrintState("After MixColumns", state);

        AddRoundKey(state, w, round);
        PrintState("After AddRoundKey", state);
    }

    // Final round (without MixColumns)
    SubBytes(state);
    PrintState("After SubBytes (Final Round)", state);

    ShiftRows(state);
    PrintState("After ShiftRows (Final Round)", state);

    AddRoundKey(state, w, Nr);
    PrintState("After AddRoundKey (Final Round)", state);

    memcpy(out, state, 4 * Nb);
}

// SubBytes transformation
void SubBytes(uint8_t *state)
{
    for (int i = 0; i < 4 * Nb; i++)
    {
        state[i] = sbox[state[i]];
    }
}

// ShiftRows transformation
void ShiftRows(uint8_t *state)
{
    uint8_t temp;

    // Rotate first row 1 columns to left
    temp = state[1];
    state[1] = state[5];
    state[5] = state[9];
    state[9] = state[13];
    state[13] = temp;

    // Rotate second row 2 columns to left
    temp = state[2];
    state[2] = state[10];
    state[10] = temp;
    temp = state[6];
    state[6] = state[14];
    state[14] = temp;

    // Rotate third row 3 columns to left
    temp = state[3];
    state[3] = state[15];
    state[15] = state[11];
    state[11] = state[7];
    state[7] = temp;
}

// MixColumns transformation
void MixColumns(uint8_t *state)
{
    uint8_t temp[4];

    for (int i = 0; i < 4; i++)
    {
        temp[0] = Multiply(0x02, state[i * 4]) ^ Multiply(0x03, state[i * 4 + 1]) ^ state[i * 4 + 2] ^ state[i * 4 + 3];
        temp[1] = state[i * 4] ^ Multiply(0x02, state[i * 4 + 1]) ^ Multiply(0x03, state[i * 4 + 2]) ^ state[i * 4 + 3];
        temp[2] = state[i * 4] ^ state[i * 4 + 1] ^ Multiply(0x02, state[i * 4 + 2]) ^ Multiply(0x03, state[i * 4 + 3]);
        temp[3] = Multiply(0x03, state[i * 4]) ^ state[i * 4 + 1] ^ state[i * 4 + 2] ^ Multiply(0x02, state[i * 4 + 3]);

        state[i * 4 + 0] = temp[0];
        state[i * 4 + 1] = temp[1];
        state[i * 4 + 2] = temp[2];
        state[i * 4 + 3] = temp[3];
    }
}

// AddRoundKey transformation
void AddRoundKey(uint8_t *state, const uint8_t *w, int round)
{
    for (int i = 0; i < 4 * Nb; i++)
    {
        state[i] ^= w[round * Nb * 4 + i];
    }
}

// Print the state matrix
void PrintState(const char *label, uint8_t *state)
{
    cout << "\n"
         << label << ":\n";
    for (int i = 0; i < 4 * Nb; i++)
    {
        if (i % 4 == 0 && i != 0)
            cout << endl;
        cout << hex << setw(2) << setfill('0') << (int)state[i] << " ";
    }
    cout << dec << endl;
}

// xtime operation for multiplication in GF(2^8)
uint8_t xtime(uint8_t x)
{
    return ((x << 1) ^ (((x >> 7) & 1) * 0x1b));
}

// Multiplication in GF(2^8)
uint8_t Multiply(uint8_t x, uint8_t y)
{
    uint8_t result = 0;
    uint8_t temp = x;

    while (y)
    {
        if (y & 1)
            result ^= temp;
        temp = xtime(temp);
        y >>= 1;
    }
    return result;
}

// Read BMP image file
bool ReadBMP(const string &filename, vector<uint8_t> &header, vector<uint8_t> &data)
{
    ifstream file(filename, ios::binary);
    if (!file)
        return false;

    // Read the BMP header (first 54 bytes)
    header.resize(54);
    file.read(reinterpret_cast<char *>(header.data()), 54);

    // Check for valid BMP file
    if (header[0] != 'B' || header[1] != 'M')
    {
        cerr << "Not a valid BMP file." << endl;
        return false;
    }

    // Get the size of the image data
    uint32_t dataOffset = *(uint32_t *)&header[10];
    uint32_t dataSize = *(uint32_t *)&header[34];
    if (dataSize == 0)
    {
        // Some BMP files may have zero in this field
        file.seekg(0, ios::end);
        dataSize = static_cast<std::streamoff>(file.tellg()) - dataOffset;
    }

    // Read the image data
    data.resize(dataSize);
    file.seekg(dataOffset, ios::beg);
    file.read(reinterpret_cast<char *>(data.data()), dataSize);

    file.close();
    return true;
}

// Write BMP image file
bool WriteBMP(const string &filename, const vector<uint8_t> &header, const vector<uint8_t> &data)
{
    ofstream file(filename, ios::binary);
    if (!file)
        return false;

    // Write the header
    file.write(reinterpret_cast<const char *>(header.data()), header.size());

    // Write the image data
    file.write(reinterpret_cast<const char *>(data.data()), data.size());

    file.close();
    return true;
}
