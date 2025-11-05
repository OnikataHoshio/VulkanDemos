#include "Utils/CommonUtils.h"

void COLOR_PRINT(std::string content, OSTREAM_TYPE OUT_TYPE, FONT_COLOR COLOR_TYPE) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, static_cast<WORD>(COLOR_TYPE));
	switch (OUT_TYPE)
	{
	case OSTREAM_TYPE::COUT:
		std::cout << content;
		break;
	case OSTREAM_TYPE::CERR:
		std::cerr << content;
		break;
	default:
		std::cout << content;
		break;
	}
	SetConsoleTextAttribute(hConsole, static_cast<WORD>(FONT_COLOR::Default));
}

void PrintProgressBar(int progress, int total, const char* tips)
{
	const int barWidth = 64; // 进度条宽度
	float ratio = static_cast<float>(progress) / total;
	int pos = static_cast<int>(barWidth * ratio);

	std::cout << std::format("\r{}:[",tips);
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) std::cout << "=";
		else if (i == pos) std::cout << ">";
		else std::cout << " ";
	}

	std::cout << "] " << int(ratio * 100.0) << " %" << std::flush;
}

glm::mat4 FlipVertical(const glm::mat4& projection)
{
	glm::mat4 _projection = projection;
	for (uint32_t i = 0; i < 4; i++)
		_projection[i][1] *= -1;
	return _projection;
}


