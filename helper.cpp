/*
This file contains "helper" functions that assist main functions in main.cpp
The printInstructions() and small utilities live here to reduce space in main.cpp
*/

#include "helper.hpp"
#include <ctime>

const aiScene* importedScene = nullptr;
Assimp::Importer importer;
bool showImportedModel = true;

GLuint modelDisplayList = 0;
bool   modelDisplayListBuilt = false;

struct TextureInfo {
    GLuint id;
    std::string type;
    std::string path;
};

std::vector<TextureInfo> modelTextures;
std::vector<Obstacle> obstacles;


// used for color menu
Vector3 getColor(Color c) {
    switch (c) {
    case RED:    return Vector3(1, 0, 0);
    case GREEN:  return Vector3(0, 1, 0);
    case BLUE:   return Vector3(0, 0, 1);
    case YELLOW: return Vector3(1, 1, 0);
    case CYAN:   return Vector3(0, 1, 1);
    case MAGENTA:return Vector3(1, 0, 1);
    case WHITE:  return Vector3(1, 1, 1);
    default:     return Vector3(1, 1, 1);
    }
}

// used for color menu
const char* getColorName(Color c) {
    switch (c) {
    case RED:     return "Red";
    case GREEN:   return "Green";
    case BLUE:    return "Blue";
    case YELLOW:  return "Yellow";
    case CYAN:    return "Cyan";
    case MAGENTA: return "Magenta";
    case WHITE:   return "White";
    default:      return "Unknown";
    }
}

// Simple bitmap string (window-space for orthographic HUD)
void drawBitmapString(float x, float y, const std::string& text, void* font) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font ? font : GLUT_BITMAP_HELVETICA_18, c);
    }
}

// instructions on using program
void printInstructions() {
    std::cout << "================= Assignment 4: Robot Hunter =================\n";
    std::cout << "Keyboard Controls:\n";
    std::cout << "  w   : Wireframe mode (entire scene)\n";
    std::cout << "  s   : Solid mode (entire scene)\n";
    std::cout << "  c   : Toggle collider display (robots + obstacles)\n";
    std::cout << "  a   : Toggle world axes at origin\n";
    std::cout << "  b   : Toggle bullet speed (slow, fast, very fast)\n";
    std::cout << "  m   : Toggle robot motion (walk / jump disruption)\n";
    std::cout << "  l/L : Toggle lighting type (directional / point)\n";
    std::cout << "  o/O : Toggle imported 3D model and shelf obstacles on/off\n";
    std::cout << "  Space : Fire bullet\n";
    std::cout << "  ESC   : Pause menu (NEW GAME / RESUME / EXIT)\n";
    std::cout << "  F1    : Toggle fullscreen / windowed\n";
    std::cout << "  F2    : Swap main FPV / ESV view\n";
    std::cout << "  F3    : Toggle game sound on/off\n";
    std::cout << "  F4    : Toggle shading (smooth Gouraud / flat)\n\n";

    std::cout << "Camera Controls:\n";
    std::cout << "  Arrow Keys (FPV main): move and rotate FPV camera\n";
    std::cout << "  Arcball (ESV main only):\n";
    std::cout << "    Left-drag  : rotate camera\n";
    std::cout << "    Right-drag : zoom in/out\n\n";

    std::cout << "Mouse Controls:\n";
    std::cout << "  ESC menu: Use arrow keys + Enter/Space or left-click buttons\n\n";

    std::cout << "Game Info:\n";
    std::cout << "  Score: +10 for each robot hit, -2 for each missed shot.\n";
    std::cout << "  Time limit: 30 seconds; eliminate all robots to complete the mission.\n";
    std::cout << "  All keyboard controls are disabled once the mission ends.\n";
    std::cout << "================================================================\n";
}



// Animation
extern bool dancing;
extern int danceAngle;
void danceTimer(int value) {
    if (dancing) {
        danceAngle = (danceAngle + 5) % 360;  // animate arms/legs rotation
        glutPostRedisplay();
        glutTimerFunc(50, danceTimer, 0);     // call again in ~50 ms
    }
}

// Draw primitive
extern State state;
void createObject(Shape type, Vector3 position, Vector3 rotation, Vector3 scale, Vector3 color)
{
    // If in wireframe mode, override color to white
    if (state == WIRE)
        color = Vector3(1.0f, 1.0f, 1.0f);

    glPushMatrix();

    // Apply transformations
    glColor3f(color.x, color.y, color.z);
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotation.x, 1, 0, 0);
    glRotatef(rotation.y, 0, 1, 0);
    glRotatef(rotation.z, 0, 0, 1);
    glScalef(scale.x, scale.y, scale.z);

    // Draw the correct object type
    if (type == CUBE)
    {
        switch (state) {
        case VERTEX:
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glPointSize(10.0f);  // creates points at all vertices
            glutSolidCube(1.0);  // solid gives real vertices
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case WIRE:
            glutWireCube(1.0);
            break;
        case SOLID:
            glutSolidCube(1.0);
            break;
        }
    }
    else if (type == SPHERE)
    {
        switch (state) {
        case VERTEX:
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glPointSize(5.0f);
            glutSolidSphere(0.5, 16, 16);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case WIRE:
            glutWireSphere(0.5, 16, 16);
            break;
        case SOLID:
            glutSolidSphere(0.5, 16, 16);
            break;
        }
    }

    glPopMatrix(); // end object creation
}

// Textured cube used for robot parts
void drawTexturedCube(GLuint tex)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    glBegin(GL_QUADS);
    // FRONT
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);

    // BACK
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);

    // LEFT
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);

    // RIGHT
    glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);

    // TOP
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.5f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);

    // BOTTOM
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, -0.5f, 0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, -0.5f, 0.5f);

    glEnd();
    glDisable(GL_TEXTURE_2D);
}

// Used to prevent overlap of primary camera and corner cameras
void clearViewportArea(int x, int y, int w, int h) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, w, h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
}

// Globals defined in main.cpp
extern std::vector<Vector3> robotPositions;
extern std::vector<float>   robotSpeeds;
extern std::vector<float>   robotOffsets;
extern std::vector<int>     robotTypes;
extern int ROBOT_COUNT;

// initializes robot randomness
void initRobots() {
    srand((unsigned int)time(NULL));
    robotPositions.resize(ROBOT_COUNT);
    robotSpeeds.resize(ROBOT_COUNT);
    robotOffsets.resize(ROBOT_COUNT);
    robotTypes.resize(ROBOT_COUNT);

    for (int i = 0; i < ROBOT_COUNT; i++) {
        // random positions in [-20,20)
        robotPositions[i] = Vector3((float)((rand() % 40) - 20), 0.0f, (float)((rand() % 40) - 20));

        // Randomize dance parameters:
        robotSpeeds[i] = 0.8f + (float)(rand() % 61) / 100.0f; // 0.8 .. 1.41
        robotOffsets[i] = (float)(rand() % 360); // phase 0..359
        robotTypes[i] = rand() % 5; // 0..4 animation type
    }
}

void drawMesh(aiMesh* mesh);

void drawNode(aiNode* node)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = importedScene->mMeshes[node->mMeshes[i]];
        drawMesh(mesh);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        drawNode(node->mChildren[i]);
    }
}

void loadModel(const char* path)
{
    importedScene = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices);

    if (!importedScene || !importedScene->mRootNode) {
        std::cout << "[ERROR] Assimp: " << importer.GetErrorString() << "\n";
        return;
    }

    std::cout << "[OK] Model loaded.\n";

    // Load all textures in the model
    modelTextures.clear();

    if (importedScene->mNumMaterials > 0)
    {
        for (unsigned int i = 0; i < importedScene->mNumMaterials; i++)
        {
            aiMaterial* material = importedScene->mMaterials[i];

            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
            {
                aiString str;
                material->GetTexture(aiTextureType_DIFFUSE, 0, &str);

                std::string texPath = str.C_Str();
                std::string full = "models/" + texPath;

                GLuint tex = loadTexture(full.c_str());

                modelTextures.push_back({ tex, "diffuse", texPath });
            }
        }
    }

    if (modelDisplayList != 0) {
        glDeleteLists(modelDisplayList, 1);
        modelDisplayList = 0;
        modelDisplayListBuilt = false;
    }

    // Build a new display list for this model
    modelDisplayList = glGenLists(1);
    glNewList(modelDisplayList, GL_COMPILE);
    drawNode(importedScene->mRootNode);
    glEndList();
    modelDisplayListBuilt = true;
}

void drawMesh(aiMesh* mesh)
{
    aiMaterial* material = importedScene->mMaterials[mesh->mMaterialIndex];

    bool hasTexture = false;

    // Bind diffuse texture if it exists
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString str;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &str);

        for (auto& t : modelTextures) {
            if (t.path == str.C_Str()) {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, t.id);
                hasTexture = true;
                break;
            }
        }
    }

    if (!hasTexture) {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.8f, 0.8f, 0.8f); // simple grey shelf if no texture
    }

    glBegin(GL_TRIANGLES);

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            unsigned int idx = face.mIndices[j];

            // Normal FIRST
            if (mesh->HasNormals()) {
                aiVector3D n = mesh->mNormals[idx];
                glNormal3f(n.x, n.y, n.z);
            }

            // UV SECOND
            if (hasTexture && mesh->HasTextureCoords(0)) {
                aiVector3D uv = mesh->mTextureCoords[0][idx];
                glTexCoord2f(uv.x, uv.y);
            }

            // Vertex LAST
            aiVector3D v = mesh->mVertices[idx];
            glVertex3f(v.x, v.y, v.z);
        }
    }

    glEnd();

    if (hasTexture) {
        glDisable(GL_TEXTURE_2D);
    }
}


void generateObstacles(int count)
{
    obstacles.clear();

    for (int i = 0; i < count; i++)
    {
        float x = static_cast<float>((rand() % 80) - 40);
        float z = static_cast<float>((rand() % 80) - 40);
        float scale = static_cast<float>(rand() % 10) / 10.0f + 1.0f;

        obstacles.emplace_back(x, 0.0f, z, scale);
    }

    std::cout << "[OK] Generated " << count << " obstacles\n";
}


void drawObstacles()
{
    if (!showImportedModel || !importedScene) return;

    for (auto& o : obstacles)
    {
        if (!o.active) continue;

        glPushMatrix();

        float s = o.scale * 4.0f;

        float yOffset = -0.2f * s;

        glTranslatef(o.x, o.y + yOffset, o.z);

        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

        glScalef(s, s, s);

        // Ensure textures show correctly
        glColor3f(1.0f, 1.0f, 1.0f);

        // Draw the imported mesh
        if (modelDisplayListBuilt) {
            glCallList(modelDisplayList);
        }
        else {
            drawNode(importedScene->mRootNode);
        }


        glPopMatrix();
    }
}



bool robotBlockedByObstacle(float robotX, float robotZ, float nextX, float nextZ)
{
    for (auto& o : obstacles)
    {
        if (!o.active) continue;

        float size = 2.0f * o.scale;

        float minX = o.x - size;
        float maxX = o.x + size;
        float minZ = o.z - size;
        float maxZ = o.z + size;

        // If next robot position would be inside shelf footprint  blocked
        if (nextX > minX && nextX < maxX &&
            nextZ > minZ && nextZ < maxZ)
        {
            return true;
        }
    }
    return false;
}
