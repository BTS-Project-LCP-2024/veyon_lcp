#include "scan.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <regex>
#include <sstream>

using namespace std;

scan::scan()
{

}


string scan::getSalle()
{
    cout << salle << endl;
    return salle;
}

string scan::getMACOutput(string nomNetbios) {
    // Open a pipe for reading the command output
    string cmd = "nbtstat -a " + nomNetbios;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return "popen failed!";
    }

    string result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        // Append the read data to the result string
        result += buffer;
    }

    // Close the pipe and wait for the command to finish
    int exitCode = pclose(pipe);

    // Check for errors in the command execution
    if (exitCode != EXIT_SUCCESS) {
        return "Command failed!";
    }

    // Remove the trailing newline character (optional)
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }

    return result;
}

vector<pair<string, string>> scan::getIPOutput() {
    vector<pair<string, string>> arpEntries;

    // Execute la commande "arp -a" et capture sa sortie
    FILE* pipe = _popen("arp -a", "r");
    if (!pipe) {
        cerr << "Error: Failed to execute 'arp -a' command." << endl;
        return arpEntries;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        string line(buffer);

        // Evite les lignes qui ne contiennent pas de paires IP-MAC
        if (line.find("Interface:") != string::npos || line.find("  ") == string::npos) {
            continue;
        }

        // Extraction des adresses IP et MAC de la ligne
        istringstream iss(line);
        string ip, mac;
        iss >> ip >> mac;

        // Converti l'adresse MAC en majuscules pour plus de cohérence
        transform(mac.begin(), mac.end(), mac.begin(), ::toupper);

        arpEntries.push_back(make_pair(ip, mac));
    }

    _pclose(pipe);
    return arpEntries;
}


void scan::scanMACIP(string nomNetbios)
{

    // Commande NBTSTAT avec le nom de l'ordinateur au choix
    string output = getMACOutput(nomNetbios);

    // Récupération de l'adresse MAC parmi le resultat de la commande avec un regex
    string macAddress;
    regex e ("([0-9A-Fa-f]{2}-){5}[0-9A-Fa-f]{2}");

    // Utilisation d'un itérateur prédéfini pour parcourir les résultats
    sregex_iterator begin(output.begin(), output.end(), e);
    sregex_iterator end;

    // Vérification si une adresse MAC a été trouvée
    if (begin != end) {
        // Affichage des correspondances
        for (sregex_iterator i = begin; i != end; ++i) {
            smatch match = *i;
            macAddress = match.str();
        }
    }

    // Récupération de l'adresse IP grâce à l'adresse MAC dans la table ARP (Par commande arp -a)
    // Transforme l'adresse MAC en majuscule pour plus de cohérence
    transform(macAddress.begin(), macAddress.end(), macAddress.begin(), ::toupper);
    vector<pair<string, string>> arpEntries = getIPOutput();

    // Trouve l'adresse IP exact avec l'adresse MAC demandé
    string ipAddress;
    for (const auto& entry : arpEntries) {
        if (entry.second == macAddress) {
            ipAddress = entry.first;
            break;
        }
    }

    std::stringstream transformed_mac;

    // Iterate over each character pair and add ":" after each pair
    for (unsigned long long i = 0; i < macAddress.length(); i += 3) {
      transformed_mac << macAddress.substr(i, 2);
      if (i < macAddress.length() - 2) {
        transformed_mac << ":";
      }
    }

    // Réponse finale du résultat de la méthode (Adresse MAC trouvée ou non)
    if (!ipAddress.empty()) {
        cout << "Ordinateur: " << nomNetbios << " | Adresse MAC: " << transformed_mac.str() << " | Adresse IP: " << ipAddress << endl;
    } else {
        cout << "L'adresse MAC " << transformed_mac.str() << " n'a pas ete trouvee." << endl;
    }
}

void scan::setSalle(string salle)
{
    this->salle=salle;
}
