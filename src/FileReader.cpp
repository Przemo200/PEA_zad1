#include "../include/FileReader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <cmath>

struct NodeCoord {
    int id;
    double x;
    double y;
};

namespace {
    std::string trim(const std::string& s) {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
            start++;
        }

        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
            end--;
        }

        return s.substr(start, end - start);
    }

    std::string toUpper(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return s;
    }

    std::string valueAfterColon(const std::string& line) {
        size_t pos = line.find(':');
        if (pos == std::string::npos) {
            return "";
        }
        return trim(line.substr(pos + 1));
    }

    int lastIntegerInLine(const std::string& line) {
        std::stringstream ss(line);
        std::string token;
        int value = -1;

        while (ss >> token) {
            try {
                value = std::stoi(token);
            } catch (...) {
            }
        }

        return value;
    }

    std::string fileNameOnly(const std::string& path) {
        size_t p1 = path.find_last_of('/');
        size_t p2 = path.find_last_of('\\');
        size_t pos = std::string::npos;

        if (p1 == std::string::npos) pos = p2;
        else if (p2 == std::string::npos) pos = p1;
        else pos = std::max(p1, p2);

        if (pos == std::string::npos) return path;
        return path.substr(pos + 1);
    }

    int distEUC2D(double x1, double y1, double x2, double y2) {
        double dx = x1 - x2;
        double dy = y1 - y2;
        double d = std::sqrt(dx * dx + dy * dy);
        return static_cast<int>(std::lround(d));
    }

    int distATT(double x1, double y1, double x2, double y2) {
        double dx = x1 - x2;
        double dy = y1 - y2;
        double rij = std::sqrt((dx * dx + dy * dy) / 10.0);
        int tij = static_cast<int>(std::lround(rij));
        if (tij < rij) {
            return tij + 1;
        }
        return tij;
    }

    std::vector<std::vector<int>> buildMatrixFromCoords(
        const std::vector<NodeCoord>& coords,
        const std::string& edgeWeightType
    ) {
        int n = static_cast<int>(coords.size());
        std::vector<std::vector<int>> matrix(n, std::vector<int>(n, 0));

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (i == j) {
                    matrix[i][j] = 0;
                    continue;
                }

                if (edgeWeightType == "EUC_2D") {
                    matrix[i][j] = distEUC2D(coords[i].x, coords[i].y, coords[j].x, coords[j].y);
                } else if (edgeWeightType == "ATT") {
                    matrix[i][j] = distATT(coords[i].x, coords[i].y, coords[j].x, coords[j].y);
                } else {
                    throw std::runtime_error("Nieobslugiwany EDGE_WEIGHT_TYPE: " + edgeWeightType);
                }
            }
        }

        return matrix;
    }

    std::vector<std::vector<int>> buildFullMatrixFromNumbers(
        int n,
        const std::vector<int>& numbers
    ) {
        if (static_cast<int>(numbers.size()) < n * n) {
            throw std::runtime_error("Za malo liczb dla FULL_MATRIX.");
        }

        std::vector<std::vector<int>> matrix(n, std::vector<int>(n, 0));
        int idx = 0;

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                matrix[i][j] = numbers[idx++];
            }
        }

        return matrix;
    }

    std::vector<std::vector<int>> buildLowerDiagRowMatrix(
    int n,
    const std::vector<int>& numbers
) {
        int needed = n * (n + 1) / 2;
        if (static_cast<int>(numbers.size()) < needed) {
            throw std::runtime_error("Za malo liczb dla LOWER_DIAG_ROW.");
        }

        std::vector<std::vector<int>> matrix(n, std::vector<int>(n, 0));
        int idx = 0;

        for (int i = 0; i < n; i++) {
            for (int j = 0; j <= i; j++) {
                int w = numbers[idx++];
                matrix[i][j] = w;
                matrix[j][i] = w;
            }
        }

        return matrix;
    }

    std::vector<std::vector<int>> buildUpperRowMatrix(
        int n,
        const std::vector<int>& numbers
    ) {
        int needed = n * (n - 1) / 2;
        if (static_cast<int>(numbers.size()) < needed) {
            throw std::runtime_error("Za malo liczb dla UPPER_ROW.");
        }

        std::vector<std::vector<int>> matrix(n, std::vector<int>(n, 0));
        int idx = 0;

        for (int i = 0; i < n; i++) {
            matrix[i][i] = 0;
            for (int j = i + 1; j < n; j++) {
                int w = numbers[idx++];
                matrix[i][j] = w;
                matrix[j][i] = w;
            }
        }

        return matrix;
    }
}

TSPInstance FileReader::loadInstance(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Nie mozna otworzyc pliku instancji: " + path);
    }

    TSPInstance instance;
    instance.name = fileNameOnly(path);
    instance.type = "UNKNOWN";
    instance.edgeWeightType = "UNKNOWN";

    std::string edgeWeightFormat = "UNKNOWN";

    std::vector<int> explicitNumbers;
    std::vector<NodeCoord> coords;

    bool inEdgeSection = false;
    bool inCoordSection = false;

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        std::string upper = toUpper(line);

        if (upper == "EOF") {
            break;
        }

        if (upper.find("NAME") == 0) {
            std::string v = valueAfterColon(line);
            if (!v.empty()) instance.name = v;
            continue;
        }

        if (upper.find("TYPE") == 0) {
            std::string v = valueAfterColon(line);
            if (!v.empty()) instance.type = toUpper(v);
            continue;
        }

        if (upper.find("DIMENSION") == 0) {
            int dim = lastIntegerInLine(line);
            if (dim > 0) instance.dimension = dim;
            continue;
        }

        if (upper.find("EDGE_WEIGHT_TYPE") == 0) {
            std::string v = valueAfterColon(line);
            if (!v.empty()) instance.edgeWeightType = toUpper(v);
            continue;
        }

        if (upper.find("EDGE_WEIGHT_FORMAT") == 0) {
            std::string v = valueAfterColon(line);
            if (!v.empty()) edgeWeightFormat = toUpper(v);
            continue;
        }

        if (upper.find("EDGE_WEIGHT_SECTION") != std::string::npos) {
            inEdgeSection = true;
            inCoordSection = false;
            continue;
        }

        if (upper.find("NODE_COORD_SECTION") != std::string::npos) {
            inCoordSection = true;
            inEdgeSection = false;
            continue;
        }

        if (inEdgeSection) {
            std::stringstream ss(line);
            int x;
            while (ss >> x) {
                explicitNumbers.push_back(x);
            }
            continue;
        }

        if (inCoordSection) {
            std::stringstream ss(line);
            NodeCoord node;
            if (ss >> node.id >> node.x >> node.y) {
                coords.push_back(node);
            }
            continue;
        }
    }

    if (instance.dimension <= 0) {
        throw std::runtime_error("Nie udalo sie odczytac DIMENSION z pliku: " + path);
    }

    if (instance.edgeWeightType == "EXPLICIT") {
        if (edgeWeightFormat == "FULL_MATRIX") {
            instance.matrix = buildFullMatrixFromNumbers(instance.dimension, explicitNumbers);
        } else if (edgeWeightFormat == "UPPER_ROW") {
            instance.matrix = buildUpperRowMatrix(instance.dimension, explicitNumbers);
        } else if (edgeWeightFormat == "LOWER_DIAG_ROW") {
            instance.matrix = buildLowerDiagRowMatrix(instance.dimension, explicitNumbers);
        } else {
            throw std::runtime_error("Nieobslugiwany EDGE_WEIGHT_FORMAT: " + edgeWeightFormat);
        }
    }
    else if (instance.edgeWeightType == "EUC_2D" || instance.edgeWeightType == "ATT") {
        if (static_cast<int>(coords.size()) != instance.dimension) {
            throw std::runtime_error("Liczba punktow w NODE_COORD_SECTION nie zgadza sie z DIMENSION.");
        }

        instance.matrix = buildMatrixFromCoords(coords, instance.edgeWeightType);
    }
    else {
        throw std::runtime_error("Nieobslugiwany format pliku. EDGE_WEIGHT_TYPE = " + instance.edgeWeightType);
    }

    return instance;
}