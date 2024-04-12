#include "engine.h"
#include <algorithm>

enum state {play, over};
state screen;


Engine::Engine() : keys() {
    this->initWindow();
    this->initShaders();
    this->initShapes();

}

Engine::~Engine() {}

unsigned int Engine::initWindow(bool debug) {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(width, height, "engine", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // OpenGL configuration
    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

    return 0;
}

void Engine::initShaders() {
    // load shader manager
    shaderManager = make_unique<ShaderManager>();

    // Load shader into shader manager and retrieve it
    shapeShader = this->shaderManager->loadShader("../res/shaders/shape.vert", "../res/shaders/shape.frag",  nullptr, "shape");

    // Configure text shader and renderer
    textShader = shaderManager->loadShader("../res/shaders/text.vert", "../res/shaders/text.frag", nullptr, "text");
    fontRenderer = make_unique<FontRenderer>(shaderManager->getShader("text"), "../res/fonts/MxPlus_IBM_BIOS.ttf", 24);

    // Set uniforms
    textShader.setVector2f("vertex", vec4(100, 100, .5, .5));
    shapeShader.use();
    shapeShader.setMatrix4("projection", this->PROJECTION);
}

void Engine::initShapes() {
    int row = 1;
    if(win_num > ROW_SIZE){
        win_num = ROW_SIZE;
    }
    //an array of squares and outlines
    for(int i = 0; i < pow(ROW_SIZE, 2); i++){
        //squares that scale their size and positions with the size of the window (so technically not squares unless the window is square
        squares.push_back(make_unique<Rect>(shapeShader, vec2((i % ROW_SIZE + 1) * (width / (ROW_SIZE + 1))/* to get the squares spaced evenly one the board */, row * (height / (ROW_SIZE + 1))), vec2(width/(2 * ROW_SIZE), height/(2 * ROW_SIZE)), color(0.5, 0.5, 0.5)));
        states.push_back(0); //populate the states array with zeros, meaning they have not been played on

        //same as squares except objects are slightly bigger to create outline effect
        //black by default to blend with background
        outlines.push_back(make_unique<Rect>(shapeShader, vec2((i % ROW_SIZE + 1) * (width / (ROW_SIZE + 1))/* to get the squares spaced evenly one the board */, row * (height / (ROW_SIZE + 1))), vec2(width/(2 * ROW_SIZE) + 10, height/(2 * ROW_SIZE) + 10), color(0, 0, 0)));
        if(i % ROW_SIZE == ROW_SIZE - 1){
            row++;
        }
    }
}

void Engine::processInput() {
    glfwPollEvents();

    // Set keys to true if pressed, false if released
    for (int key = 0; key < 1024; ++key) {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            keys[key] = true;
        else if (glfwGetKey(window, key) == GLFW_RELEASE)
            keys[key] = false;
    }

    // Close window if escape key is pressed
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &MouseX, &MouseY);


    // Mouse position is inverted because the origin of the window is in the top left corner
    MouseY = height - MouseY; // Invert y-axis of mouse position
    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    //for each square, if it was clicked toggle it and adjacent squares.
    for(int i = 0; i < pow(ROW_SIZE, 2); i++){
        if(screen == play && mousePressedLastFrame && !mousePressed && squares[i]->isOverlapping(vec2(MouseX, MouseY))){
            //if the state of a square is 0, it has not been played on yet and is playable
            //if it is player 1's turn and a playable square is clicked, that square's state changes to 1 and it becomes player 2's turn
            //if it is player 2's turn and a playable square is clicked, that square's state changes to 2 and it becomes player 1's turn
            if(states[i] == 0){
                if(player == 1){
                    states[i] = 1;
                    player = 2;
                }
                else{
                    states[i] = 2;
                    player = 1;
                }
            }

            //check for a win
            //TODO: create this algorithm :(
            //need to check horizontal, vertical, and diagonals

        }
        //add a red outline if a square is moused over
        if(screen == play && player == 1 && squares[i]->isOverlapping(vec2(MouseX,MouseY))){
            outlines[i]->setColor(vec3(1,0,0));
        }
        else if(screen == play && player == 2 && squares[i]->isOverlapping(vec2(MouseX,MouseY))){
            outlines[i]->setColor(vec3(0,0,1));
        }
        else{
            outlines[i]->setColor(vec3(0,0,0));
        }

    }
    // Save mousePressed for next frame
    mousePressedLastFrame = mousePressed;
}

void Engine::update() {
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    //compute the sum of all elements in the states array
    int sum = 0;
    for(int num : states){
        sum += num;
    }

    //if the sum is 0, then all lights are off and the game is over
    if(sum == 0){
        //screen = over;
    }

    //change the color of each square depending on its state
    for(int i = 0; i < pow(ROW_SIZE, 2); i++){
        if(states[i] == 1){
            squares[i]->setColor(vec3(1,0,0));
        }
        else if(states[i] == 2){
            squares[i]->setColor(vec3(0,0,1));
        }
        else{
            squares[i]->setColor(vec3(.5,.5,.5));
        }
    }

}

void Engine::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color
    glClear(GL_COLOR_BUFFER_BIT);

    // Set shader to use for all shapes
    shapeShader.use();

    // Render differently depending on screen
    switch (screen) {
        case play: {

            for(unique_ptr<Shape>& o: outlines){
                o->setUniforms();
                o->draw();
            }
            for(unique_ptr<Shape>& s : squares){
                s->setUniforms();
                s->draw();
            }
            break;
        }
        case over: {
            //render the board
            for(unique_ptr<Shape>& s : squares){
                s->setUniforms();
                s->draw();
            }
            string message = "You win!";

            //Display the message on the screen
            //for some reason the position of the text doesn't scale properly with window size, but stays at the same place with static numbers
            //the original values for position are as follows:
            //x: width/2 - (12 * message.length())
            //y: height/2
            //these values only make the text centered when the window is 800 X 600
            //I replaced width/2 and height/2 with 400 and 300, which are what they would be when the window is 800x600
            //for some reason these numbers make the text always centered at every window size
            this->fontRenderer->renderText(message, 400 - (12 * message.length()), 300, 1, vec3{1, 1, 1});
            break;
        }
    }

    glfwSwapBuffers(window);
}


bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}

GLenum Engine::glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        cout << error << " | " << file << " (" << line << ")" << endl;
    }
    return errorCode;
}