#include <algorithm>
#include <filesystem>
#include <system_error>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int inline invert(int x){
	return 255-x;
}
int inline cutoff(int x){
	if(x<0)
		return 0;
	else if(x>255)
		return 255;
	return x;
}

void exitIf(bool failureCondition, const char errorMessage[]) {
  if(failureCondition) {
    std::cerr << "Error: " << errorMessage;
    std::exit(EXIT_FAILURE);
  }
}
template <typename numberType>
void parseLittleEndian(ifstream& file, numberType& var, int size) {
  var=file.get();
  for(int i=1, f=1; i<size; i++) {
    f*=256;
    unsigned char c = file.get();
    exitIf(file.eof(), "File is too small.");
    var += f*c;
  }
}
char* readFromFile(const char fileName[], int& bitsPerPixel, long& width, long& height) {
  ifstream file(fileName, ios::binary);
  
  //parse header
  //parse bitmap header
  //parse file signature
  exitIf(file.get() != 'B' || file.get() != 'M', "Not a bitmap file.");
  
  unsigned long fileSize, offset, dibHeaderSize, compression, imageSize, colorPalette;
  int colorPlanes;

  parseLittleEndian(file, fileSize, 4);

  //skip fields
  file.ignore(4);

  parseLittleEndian(file, offset, 4);
    
  //parse DIB header
  parseLittleEndian(file, dibHeaderSize, 4);
  exitIf(dibHeaderSize+14 != offset, "Inconsistent header data.");

  parseLittleEndian(file, width, 4);
	exitIf(width <= 0, "Non-positive width not supported.");
  parseLittleEndian(file, height, 4);
	exitIf(height <= 0, "Non-positive height not supported.");

  parseLittleEndian(file, colorPlanes, 2);
  exitIf(colorPlanes != 1, "Can't process more than one color plane.");

  parseLittleEndian(file, bitsPerPixel, 2);
  exitIf(bitsPerPixel != 24, "Can only process files with a color depth of 24 bits per pixel.");

  parseLittleEndian(file, compression, 4);
  exitIf(compression != 0, "Can't process compressed files.");

  parseLittleEndian(file, imageSize, 4);
  exitIf(imageSize != (bitsPerPixel*width+31)/32*4*height, "Inconsistent image dimensions in header.");

  file.ignore(8);

  parseLittleEndian(file, colorPalette, 4);
  exitIf(colorPalette != 0, "Can't process color palettes.");

  file.ignore(offset-50);

  //read image data
  char* data = new char[imageSize];
  file.read(data, imageSize);

  file.close();

	return data;
}
void createFileHeader(char fileHeader[], int bitsPerPixel, unsigned long width, unsigned long height) {
	//Bitmap & DIB header (54 bytes)

	//Bitmap file header (14 bytes)

	//'BM' identifies the BMP and DIB file (2 bytes)
	fileHeader[0] = 'B';
	fileHeader[1] = 'M';

	unsigned long imageSize = (bitsPerPixel*width+31)/32*4*height;
	unsigned long fileSize = 54 + imageSize;
	//size of the BMP file in bytes (4 bytes)
	fileHeader[2] = fileSize % 256;
	fileSize /= 256;
	fileHeader[3] = fileSize % 256;
	fileSize /= 256;
	fileHeader[4] = fileSize % 256;
	fileHeader[5] = fileSize / 256;

	//reserved; if created manually, can be 0 (2 bytes)
	fileHeader[6] = (char)0;
	fileHeader[7] = (char)0;

	//reserved; if created manually, can be 0 (2 bytes)
	fileHeader[8] = (char)0;
	fileHeader[9] = (char)0;

	//offset to data (4 bytes)
	fileHeader[10] = (char)54;
	fileHeader[11] = (char)0;
	fileHeader[12] = (char)0;
	fileHeader[13] = (char)0;

	//DIB header (bitmap information header) (different formats)
	//simplest common format is BITMAPINFOHEADER (40 bytes)

	//header size (4 bytes)
	fileHeader[14] = (char)40;
	fileHeader[15] = (char)0;
	fileHeader[16] = (char)0;
	fileHeader[17] = (char)0;

	//bitmap width in pixels (signed integer) (4 bytes)
	fileHeader[18] = width % 256;
	width /= 256;
	fileHeader[19] = width % 256;
	width /= 256;
	fileHeader[20] = width % 256;
	fileHeader[21] = width / 256;

	//bitmap height in pixels (signed integer) (4 bytes)
	fileHeader[22] = height % 256;
	height /= 256;
	fileHeader[23] = height % 256;
	height /= 256;
	fileHeader[24] = height % 256;
	fileHeader[25] = height / 256;

	//number of color planes (must be 1) (2 bytes)
	fileHeader[26] = (char)1;
	fileHeader[27] = (char)0;

	//number of bits per pixel (2 bytes)
	fileHeader[28] = (char)bitsPerPixel;
	fileHeader[29] = (char)0;

	//compression method (4 bytes)
	//a value of '0' means no compression
	fileHeader[30] = (char)0;
	fileHeader[31] = (char)0;
	fileHeader[32] = (char)0;
	fileHeader[33] = (char)0;

	//image size in bytes (without header) (4 bytes)
	fileHeader[34] = imageSize % 256;
	imageSize /= 256;
	fileHeader[35] = imageSize % 256;
	imageSize /= 256;
	fileHeader[36] = imageSize % 256;
	fileHeader[37] = imageSize / 256;

	//(rest isn't needed)

	//horizontal resolution (pixels per metre, signed integer) (4 bytes)
	fileHeader[38] = (char)0;
	fileHeader[39] = (char)0;
	fileHeader[40] = (char)0;
	fileHeader[41] = (char)0;

	//vertical resolution (pixels per metre, signed integer) (4 bytes)
	fileHeader[42] = (char)0;
	fileHeader[43] = (char)0;
	fileHeader[44] = (char)0;
	fileHeader[45] = (char)0;

	//number of colors in the color palette, '0' defaults to 2**n (4 bytes)
	fileHeader[46] = (char)0;
	fileHeader[47] = (char)0;
	fileHeader[48] = (char)0;
	fileHeader[49] = (char)0;

	//number of important colors used, or '0' when every color is important (4 bytes)
	fileHeader[50] = (char)0;
	fileHeader[51] = (char)0;
	fileHeader[52] = (char)0;
	fileHeader[53] = (char)0;
}
void writeToFile(const char fileName[], int bitsPerPixel, unsigned long width, unsigned long height, char data[]) {
  char fileHeader[54];
  createFileHeader(fileHeader, bitsPerPixel, width, height);
  ofstream file(fileName, ios::binary);
	file.write(fileHeader, 54);
	file.write(data, (bitsPerPixel*width+31)/32*4*height);
	file.close();
}

int main(int argc, char* argv[]){
  exitIf(argc==1, "No image provided for processing.");
	std::filesystem::path filePath(argv[1]);
	std::filesystem::path basePath(filePath);
	basePath.remove_filename();

	int bitsPerPixel;
  long width, height;
	char *originalFileData, *changedFileData;
	
  originalFileData = readFromFile(filePath.string().c_str(), bitsPerPixel, width, height);
	const int rowSize = (bitsPerPixel*width+31)/32*4;

	#define color(buffer, x, y, color) buffer[rowSize*y+3*x+color]
	//pixel colors are stored in the order blue → green → red
	enum {BLUE, GREEN, RED};
	int currentPermutation[] = {BLUE, GREEN, RED};
	const char* 			 map[] = {"B", "G", "R"};
	const char* inverseMap[] = {"Bi", "Gi", "Ri"};

	std::filesystem::file_type::directory;
	for(const char* s : {"Permuted", "Inverted", "Inverted and permuted"}) {
		std::filesystem::path newDirectoryPath(basePath);
		newDirectoryPath /= s;

		std::error_code ec;
		std::filesystem::create_directory(newDirectoryPath, ec);
		exitIf(static_cast<bool>(ec), ec.message().c_str());
	}

	for(int p=0; p<6; p++) { //permutations
		for(int r=0; r<2; r++) { //invert red?
			for(int g=0; g<2; g++) { //invert green?
				for(int b=0; b<2; b++) { //invert blue?
					changedFileData = new char[rowSize*height];

					std::string folder = r||g||b?(p==0?"Inverted":"Inverted and permuted"):"Permuted";

					std::filesystem::path newFilePath(filePath);
					newFilePath.replace_filename(folder);
					newFilePath /= filePath.stem().string() + '_' +
					  (r ? inverseMap[currentPermutation[RED]]   : map[currentPermutation[RED]]  ) +
					  (g ? inverseMap[currentPermutation[GREEN]] : map[currentPermutation[GREEN]]) +
					  (b ? inverseMap[currentPermutation[BLUE]]  : map[currentPermutation[BLUE]] ) +
						filePath.extension().string();

					for(int y=0; y<height; y++) {
						for(int x=0; x<width; x++) {
							for(int c=0; c<3; c++) { //colors

								switch(c) {
									case RED:
										color(changedFileData, x, y, currentPermutation[c]) = r ? invert(color(originalFileData, x, y, c)) : color(originalFileData, x, y, c);
									break;
									case GREEN:
										color(changedFileData, x, y, currentPermutation[c]) = g ? invert(color(originalFileData, x, y, c)) : color(originalFileData, x, y, c);
									break;
									case BLUE:
										color(changedFileData, x, y, currentPermutation[c]) = b ? invert(color(originalFileData, x, y, c)) : color(originalFileData, x, y, c);
									break;
								}

							}
						}
						//with 24-bit bitmaps, padding of width%4 bytes needs to be added to the end of each row
						for(int i=0; i<width%4; i++) {
							color(changedFileData, width, y, i) = 0;
						}
					}

					writeToFile(newFilePath.string().c_str(), bitsPerPixel, width, height, changedFileData);

					delete [] changedFileData;
				}
			}
		}
		std::next_permutation(currentPermutation, currentPermutation+3);
	}

	delete [] originalFileData;

	return 0;
}
