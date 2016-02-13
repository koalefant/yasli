#pragma once

struct TreeConfig
{
	TreeConfig();

	static TreeConfig defaultConfig;

	bool fullRowContainers;
	bool immediateUpdate;
	bool hideUntranslated;
	bool showContainerIndices;
	bool filterWhenType;
	float valueColumnWidth;
	int filter;
	int tabSize;
	int sliderUpdateDelay;
	int defaultRowHeight;

	int expandLevels;
	bool undoEnabled;
	bool fullUndo;
	bool multiSelection;

	// debug tools
	bool debugDrawLayout;
};
