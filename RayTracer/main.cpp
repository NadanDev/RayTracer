#include <iostream>

using namespace std;

int main()
{
	int imageWidth = 256;
	int imageHeight = 256;

	cout << "P3\n" << imageWidth << ' ' << imageHeight << "\n255\n";

	for (int j = 0; j < imageHeight; j++)
	{
		clog << "\rScanlines remaining: " << (imageHeight - j) << '\n';
		for (int i = 0; i < imageWidth; i++)
		{
			auto r = double(i) / (imageWidth - 1);
			auto g = double(j) / (imageHeight - 1);
			auto b = 0.0;

			int ir = int(255.999 * r); // Avoid truncation errors
			int ig = int(255.999 * g);
			int ib = int(255.999 * b);

			cout << ir << ' ' << ig << ' ' << ib << "\n";
		}
	}

	clog << "Done.\n";
}