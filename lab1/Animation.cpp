
#include "FullName.h"
#include "LED42FontDrawer.h"
#include "Viselitsa.h"
#include "MVVM_VibeClicker.h"

#include <iostream>

static auto RunSolve1() -> void
{
	LettersManager manager{};
	manager.RunAnimation();
}

static auto RunSolve2() -> void
{
	RenderLED42Message(BuildMessageByFile("input.txt"));
}

static auto RunSolve3() -> void
{
	RunViselitsa();
}

static auto RunClickerMVVM() -> void
{
	RunClicker();
}

int main()
{
	setlocale(LC_ALL, "Rus");
	RunClickerMVVM();

	return EXIT_SUCCESS;
}
