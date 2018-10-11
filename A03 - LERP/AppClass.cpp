#include "AppClass.h"
void Application::InitVariables(void)
{
	//Change this to your name and email
	m_sProgrammer = "Cobey Adekanbi - cja1093@rit.edu";
	
	//Set the position and target of the camera
	//(I'm at [0,0,10], looking at [0,0,0] and up is the positive Y axis)
	m_pCameraMngr->SetPositionTargetAndUpward(AXIS_Z * 20.0f, ZERO_V3, AXIS_Y);

	//if the light position is zero move it
	if (m_pLightMngr->GetPosition(1) == ZERO_V3)
		m_pLightMngr->SetPosition(vector3(0.0f, 0.0f, 3.0f));

	//if the background is cornflowerblue change it to black (its easier to see)
	if (vector3(m_v4ClearColor) == C_BLUE_CORNFLOWER)
	{
		m_v4ClearColor = vector4(ZERO_V3, 1.0f);
	}
	
	// Index vector for keeping track of each circles lerp targets
	for (int i = 0; i < m_uOrbits; i++) {
		m_vIndexes.push_back(0);
	}

	//if there are no segments create 7
	if(m_uOrbits < 1)
		m_uOrbits = 7;

	float fSize = 1.0f; //initial size of orbits

	//creating a color using the spectrum 
	uint uColor = 650; //650 is Red
	//prevent division by 0
	float decrements = 250.0f / (m_uOrbits > 1? static_cast<float>(m_uOrbits - 1) : 1.0f); //decrement until you get to 400 (which is violet)

	// Create orbits
	// -------------
	uint uSides = 3; //start with the minimal 3 sides
	for (uint i = uSides; i < m_uOrbits + uSides; i++)
	{
		vector3 v3Color = WaveLengthToRGB(uColor); //calculate color based on wavelength
		m_shapeList.push_back(m_pMeshMngr->GenerateTorus(fSize, fSize - 0.1f, 3, i, v3Color)); //generate a custom torus and add it to the meshmanager
		
		std::vector<vector3> tempVec;
		// Generate Locations for stop list
		for (int j = 0; j < i; j++) {
			 tempVec.push_back(vector3(cos((2 * PI / i) * j), sin((2 * PI / i) * j), 0.0) * ((i + 2) / uSides));
		}
		// Set give list to list of stoplists
		m_stopsList.push_back(tempVec);
		std::cout << tempVec[0].x << std::endl;
		m_currentPositions.push_back(m_stopsList[i - uSides][0]);
		
		fSize += 0.5f; //increment the size for the next orbit
		uColor -= static_cast<uint>(decrements); //decrease the wavelength
	}
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();
	
	// Timer magic
	static float fTimer = 0;
	static uint uClock = m_pSystem->GenClock();
	fTimer += m_pSystem->GetDeltaTime(uClock);
	static float fPercentage = MapValue(fTimer, 0.0f, 2.0f, 0.0f, 1.0f);

	// Start & End Positions
	vector3 start;
	vector3 end;

	matrix4 m4View = m_pCameraMngr->GetViewMatrix(); //view Matrix
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix(); //Projection Matrix
	matrix4 m4Offset = IDENTITY_M4; //offset of the orbits, starts as the global coordinate system

	//m4Offset = glm::rotate(IDENTITY_M4, 1.5708f, AXIS_Z);

	// draw a shapes
	for (uint i = 0; i < m_uOrbits; ++i)
	{
		// Reset when percentage is >= 1.0f
		if (fPercentage >= 1.0f) {
			m_vIndexes[i] += 1;
			fTimer = m_pSystem->GetDeltaTime(uClock);
			m_vIndexes[i] %= m_stopsList[0].size();
		}

		// Set start end values for this current iteration
		start = m_stopsList[i].at(m_vIndexes[i]);
		if (m_vIndexes[i] >= m_stopsList[i].size() - 1) {
			m_vIndexes[i] = 0;
		}
		else {
			m_vIndexes[i]++;
		}
		end = m_stopsList[i].at(m_vIndexes[i]);
		
		// Lerp (Still messin up)
		m_currentPositions[i] = glm::lerp(start, end, fPercentage);

		// Add mesh to render list
		m_pMeshMngr->AddMeshToRenderList(m_shapeList[i], glm::rotate(m4Offset, 1.5708f, AXIS_X));

		//calculate the current position
		vector3 v3CurrentPos = m_currentPositions[i];
		matrix4 m4Model = glm::translate(m4Offset, v3CurrentPos);

		//draw spheres
		m_pMeshMngr->AddSphereToRenderList(m4Model * glm::scale(vector3(0.1)), C_WHITE);
	}

	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	//release GUI
	ShutdownGUI();
}