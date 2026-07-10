#include <windows.h>
// 1. Incluimos la librería para el manejo del diseño del sistema
#include <dwmapi.h>

// Ventana principal
#define CENTRO_X 450
#define CENTRO_Y 100
#define ANCHO_APP 340
#define ALTO_APP 565
// Pantalla de la calculadora y botones
#define ID_PANTALLA 200
#define ID_BOTON_BASE 300
// Colores de los botones de la calculadora
#define COL_BTN_OPER  RGB(45, 45, 45)
#define COL_BTN_NUMS RGB(50, 50, 50)
#define COL_EQUAL_BTN RGB(0, 130, 225)

// Variables para rastrear el hover
WNDPROC ProcedimientoOriginalBoton;
HWND botonHovered = NULL;

// El "Espía" (Subclass Procedure) para los botones
LRESULT CALLBACK BotonHoverProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_MOUSEMOVE: {
			// Si el mouse acaba de entrar a este botón
			if (botonHovered != hwnd) {
				botonHovered = hwnd;
				
				// Forzamos al botón a que se vuelva a dibujar inmediatamente
				InvalidateRect(hwnd, NULL, FALSE); 

				// Le pedimos a Windows que nos avise cuando el mouse SALGA del botón
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hwnd;
				TrackMouseEvent(&tme);
			}
			break;
		}
		case WM_MOUSELEAVE: {
			// El mouse salió del botón
			botonHovered = NULL;
			
			// Forzamos al botón a que se dibuje de nuevo (para quitar el color claro)
			InvalidateRect(hwnd, NULL, FALSE); 
			break;
		}
	}
	// Dejamos que el botón haga el resto de sus cosas normales (como detectar clics)
	return CallWindowProc(ProcedimientoOriginalBoton, hwnd, msg, wParam, lParam);
}

/* This is where all the input to the window goes to */
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	
	static HFONT fuenteGrande;
	
	switch(Message) {
		/* WM_CREATE se ejecuta una sola vez al nacer la ventana */
		case WM_CREATE: {
			// 1. Creamos la pantalla (la hacemos más alta para igualar a Windows)
			HWND hwndPantalla = CreateWindowEx(0, "EDIT", "1.234.567.890", 
				WS_CHILD | WS_VISIBLE |  WS_CHILDWINDOW | ES_RIGHT | ES_READONLY,
				0, 73, 320, 77, /* x, y, ancho, alto */
				hwnd, (HMENU)ID_PANTALLA, NULL, NULL);

			fuenteGrande = CreateFont(48, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, 
								  DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, 
								  CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
								  VARIABLE_PITCH, "Segoe UI");

			SendMessage(hwndPantalla, WM_SETFONT, (WPARAM)fuenteGrande, TRUE);

			// 2. Definimos los textos de los botones exactamente como en tu imagen
			// Usamos 24 elementos (6 filas x 4 columnas)
			// LPCWSTR significa "Puntero a texto Unicode constante"
			LPCWSTR botones[24] = {
				L"%", L"CE", L"C", L"\x232B",    // \x232B es el símbolo ? (Borrar)
				L"1/x", L"x\x00B2", L"\x221Ax", L"\x00F7", // x², vx, ÷
				L"7", L"8", L"9", L"\x00D7",     // \x00D7 es el símbolo ×
				L"4", L"5", L"6", L"-",
				L"1", L"2", L"3", L"+",
				L"+/-", L"0", L",", L"="
			};

			// 3. Variables para la matemática de la cuadrícula
			int margenX = 4;      // Espacio desde el borde izquierdo
			int margenY = 185;     // Espacio desde el borde superior (dejamos espacio para la pantalla)
			int anchoBtn = 77;     // Ancho de cada botón
			int altoBtn = 54;      // Alto de cada botón
			int espacio = 2;       // Separación entre botones

			// Fuente normal para los números
			HFONT fuenteBotones = CreateFontW(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
								  DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, 
								  CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");

			// NUEVA: Fuente de símbolos de interfaz de Windows
			// Tamaño 22 se ajusta perfectamente a un botón de 54px de alto
			HFONT fuenteMath = CreateFontW(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
								  DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, 
								  CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI Symbol");

			// 4. El bucle mágico que crea los 24 botones automáticamente
			int i;
			for ( i = 0; i < 24; i++) {
				// Calculamos la fila y columna actual
				int columna = i % 4;
				int fila = i / 4;

				// Calculamos las coordenadas exactas X e Y
				int x = margenX + columna * (anchoBtn + espacio);
				int y = margenY + fila * (altoBtn + espacio);

				
				// Cambiamos a BS_OWNERDRAW para pintarlos nosotros
				DWORD estilo = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW;
				
				// Creamos el botón
				HWND btn = CreateWindowExW(0, L"BUTTON", botones[i], estilo,
					x, y, anchoBtn, altoBtn,
					hwnd, (HMENU)(INT_PTR)(ID_BOTON_BASE + i), NULL, NULL);

				// --- MODIFICACIÓN: Aplicamos la fuente según el tipo de botón ---
				// Usamos la misma regla de tus colores: Botones especiales o columna derecha
				if (i < 8 || i % 4 == 3) {
					// A los operadores les ponemos Cambria Math
					SendMessage(btn, WM_SETFONT, (WPARAM)fuenteMath, TRUE);
				} else {
					// A los números les ponemos Segoe UI
					SendMessage(btn, WM_SETFONT, (WPARAM)fuenteBotones, TRUE);
				}
				// ----------------------------------------------------------------

				// Inyectamos nuestro espía al botón (tu código de hover)
				ProcedimientoOriginalBoton = (WNDPROC)SetWindowLongPtr(btn, GWLP_WNDPROC, (LONG_PTR)BotonHoverProc);
			}
			break;
		}
		
		/* ESTE MENSAJE PINTA LA PANTALLA (FONDO Y TEXTO) */
		case WM_CTLCOLORSTATIC: {
			HDC hdc = (HDC)wParam; // Obtenemos el "pincel" de la pantalla
			
			// Verificamos que sea nuestra pantalla y no otro control
			if ((HWND)lParam == GetDlgItem(hwnd, ID_PANTALLA)) {
				SetTextColor(hdc, RGB(255, 255, 255)); // Texto de color blanco
				SetBkColor(hdc, RGB(32, 32, 32));      // Fondo del texto gris oscuro
				
				// Creamos una brocha gris oscura para el fondo del rectángulo
				static HBRUSH brochaFondo = NULL;
				if (!brochaFondo) brochaFondo = CreateSolidBrush(RGB(32, 32, 32));
				
				return (INT_PTR)brochaFondo;
			}
			return DefWindowProc(hwnd, Message, wParam, lParam);
		}

		/* ESTE MENSAJE PINTA NUESTROS BOTONES PERSONALIZADOS */
		case WM_DRAWITEM: {
			LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
			
			// Nos aseguramos de que lo que estamos pintando es un botón
			if (pdis->CtlType == ODT_BUTTON) {
				int idBoton = pdis->CtlID - ID_BOTON_BASE; // Del 0 al 23
				
				// 1. Elegir colores (como en Windows 10/11)
				COLORREF colorFondo = COL_BTN_NUMS;   // Gris estándar para números
				COLORREF colorTexto = RGB(255, 255, 255); // Blanco
				
				// Si es un botón de operación (arriba a la derecha), lo hacemos un poco más oscuro
				if (idBoton < 8 || idBoton % 4 == 3) {
					colorFondo = COL_BTN_OPER; 
				}
				
				// Si es el botón "=" (el último), lo pintamos azul
				if (idBoton == 23) {
					colorFondo = COL_EQUAL_BTN; // Azul estilo Windows
					colorTexto = RGB(0, 0, 0); // Negro
				}
					
				// Si el usuario tiene el clic presionado, oscurecemos un poco
				if (pdis->itemState & ODS_SELECTED) {
					if(idBoton == 23){
						colorFondo = RGB(0, 0, 0);	
					}else{
						colorFondo = RGB(80, 80, 80);
					}	 
				} 
				// Si no está haciendo clic, pero el mouse está encima (Hover)
				else if(pdis->hwndItem == botonHovered) {
				    if (idBoton == 23) {
				        colorFondo = RGB(25, 115, 255);
				        colorTexto = RGB(10, 10, 10);
				    } else {
				        if (idBoton < 8 || idBoton % 4 == 3) {
							colorFondo = COL_BTN_NUMS; // Los botones de operaciones se vuelven del color de los de numeros 
						} else {
				            colorFondo = COL_BTN_OPER; // Los botones de numeros se vuelven del color de las operaciones
				        }    
				    }
				}

				// 2. Pintar el fondo del botón
				HBRUSH brocha = CreateSolidBrush(colorFondo);
				FillRect(pdis->hDC, &pdis->rcItem, brocha);
				DeleteObject(brocha);

				// 3. Pintar el texto encima
				SetTextColor(pdis->hDC, colorTexto);
				SetBkMode(pdis->hDC, TRANSPARENT); // Fondo del texto transparente
				
				// --- EL TRUCO PARA LOS SÍMBOLOS ---
				// 1. Le pedimos al botón la fuente que le asignamos en el WM_CREATE
				HFONT fuenteDelBoton = (HFONT)SendMessage(pdis->hwndItem, WM_GETFONT, 0, 0);
				
				// 2. Se la asignamos temporalmente a nuestro pincel (hDC)
				// Guardamos la fuente vieja para ser educados con la memoria de Windows
				HFONT fuenteVieja = (HFONT)SelectObject(pdis->hDC, fuenteDelBoton);
				// ----------------------------------
				
				// Obtenemos el texto del botón y lo dibujamos centrado
				wchar_t textoBoton[10];
				GetWindowTextW(pdis->hwndItem, textoBoton, 10);
				DrawTextW(pdis->hDC, textoBoton, -1, &pdis->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				
				// 3. Restauramos el pincel a su estado original al terminar
				SelectObject(pdis->hDC, fuenteVieja);
				
				return TRUE; // Le decimos a Windows: "Ya lo pinté yo"
			}
			break;
		}
		/* Upon destruction, tell the main thread to stop */
		case WM_DESTROY: {
			// Es una buena práctica de programación liberar la memoria de la fuente al cerrar
			DeleteObject(fuenteGrande); 
			PostQuitMessage(0);
			break;
		}
		
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

/* The 'main' function of Win32 GUI programs: this is where execution starts */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc; /* A properties struct of our window */
	HWND hwnd; /* A 'HANDLE', hence the H, or a pointer to our window */
	MSG msg; /* A temporary location for all messages */

	/* zero out the struct and set the stuff we want to modify */
	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc; /* This is where we will send messages to */
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	
	/* White, COLOR_WINDOW is just a #define for a system color, try Ctrl+Clicking it */
	wc.hbrBackground = CreateSolidBrush(RGB(32, 32, 32));
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIcon(hInstance, MAKEINTRESOURCE(101)); /* Load a standard icon */
	wc.hIconSm		 = LoadIcon(hInstance, MAKEINTRESOURCE(101)); /* use the name "A" to use the project icon */

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass","Calculadora de Uli",WS_OVERLAPPEDWINDOW|WS_OVERLAPPEDWINDOW,
		CENTRO_X, /* x */
		CENTRO_Y, /* y */
		ANCHO_APP, /* width */
		ALTO_APP, /* height */
		NULL,NULL,hInstance,NULL);

	if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONHAND|MB_OK);
		return 0;
	}
	
	// --- CONFIGURACIÓN PARA WINDOWS 10 (Modo Oscuro Inteligente) ---
	BOOL usarModoOscuro = TRUE;
	
	// Intentamos aplicar el modo oscuro con el atributo 20 (Win10 nuevos/Win11)
	// HRESULT es un tipo de dato que guarda si la operación fue un éxito o falló
	HRESULT resultado = DwmSetWindowAttribute(hwnd, 20, &usarModoOscuro, sizeof(usarModoOscuro));
	
	// Si falló (FAILED), significa que tienes un Windows 10 más antiguo, así que probamos el 19
	if (FAILED(resultado)) {
		DwmSetWindowAttribute(hwnd, 19, &usarModoOscuro, sizeof(usarModoOscuro));
	}
	// ---------------------------------------------------
	
	// 2do CAMBIO: Ahora que ya la pintamos de oscuro, le decimos a Windows que la muestre
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd); // Forzamos a que se dibuje inmediatamente
	
	/*
		This is the heart of our program where all input is processed and 
		sent to WndProc. Note that GetMessage blocks code flow until it receives something, so
		this loop will not produce unreasonably high CPU usage
	*/
	while(GetMessage(&msg, NULL, 0, 0) > 0) { /* If no error is received... */
		TranslateMessage(&msg); /* Translate key codes to chars if present */
		DispatchMessage(&msg); /* Send it to WndProc */
	}
	return msg.wParam;
}
