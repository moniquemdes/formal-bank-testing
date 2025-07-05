#include <assert.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

#include "bank.hpp"

using json = nlohmann::json;
using namespace std;

enum class Action {
  Init,
  Deposit,
  Withdraw,//saque
  Transfer,//transferencia
  BuyInvestment,
  SellInvestment,
  Unknown
};

Action stringToAction(const std::string &actionStr) {
  static const std::unordered_map<std::string, Action> actionMap = {
      {"init", Action::Init},
      {"deposit_action", Action::Deposit},
      {"withdraw_action", Action::Withdraw},
      {"transfer_action", Action::Transfer},
      {"buy_investment_action", Action::BuyInvestment},
      {"sell_investment_action", Action::SellInvestment}};

  auto it = actionMap.find(actionStr);
  if (it != actionMap.end()) {
    return it->second;
  } else {
    return Action::Unknown;
  }
}


int int_from_json(json j) {
  string s = j["#bigint"];
  return stoi(s);
}

map<string, int> balances_from_json(json j) {
  map<string, int> m;
  for (auto it : j["#map"]) {
    m[it[0]] = int_from_json(it[1]);
  }
  return m;
}

map<int, Investment> investments_from_json(json j) {
  map<int, Investment> m;
  for (auto it : j["#map"]) {
    m[int_from_json(it[0])] = {.owner = it[1]["owner"],
                               .amount = int_from_json(it[1]["amount"])};
  }
  return m;
}

BankState bank_state_from_json(json state) {
  map<string, int> balances = balances_from_json(state["balances"]);
  map<int, Investment> investments =
      investments_from_json(state["investments"]);
  int next_id = int_from_json(state["next_id"]);
  return {.balances = balances, .investments = investments, .next_id = next_id};
}

void print_bank_state(const BankState &bank_state) {
  cout << "BankState {" << endl;
  cout << "  balances: {" << endl;
  for (const auto &it : bank_state.balances) {
      cout << "    " << it.first << ": " << it.second << endl;
  }
  cout << "  }" << endl;
  cout << "  investments: {" << endl;
  for (const auto &it : bank_state.investments) {
      cout << "    " << it.first << ": { owner: " << it.second.owner
           << ", amount: " << it.second.amount << " }" << endl;
  }
  cout << "  }" << endl;
  cout << "  next_id: " << bank_state.next_id << endl;
  cout << "}" << endl;
}


int main() {
  for (int i = 0; i < 10000; i++) {
    cout << "Trace #" << i << endl;
    std::ifstream f("traces/out" + to_string(i) + ".itf.json");
    json data = json::parse(f);

    // Estado inicial: começamos do mesmo estado incial do trace
    BankState bank_state =
        bank_state_from_json(data["states"][0]["bank_state"]);

    auto states = data["states"];
    for (auto state : states) {
      string action = state["mbt::actionTaken"];
      json nondet_picks = state["mbt::nondetPicks"];

      string error = "";

      // Próxima transição
      switch (stringToAction(action)) {
      case Action::Init: {
        cout << "initializing" << endl;
        break;
      }
      case Action::Deposit: {
        string depositor = nondet_picks["depositor"]["value"];
        int amount = int_from_json(nondet_picks["amount"]["value"]);
        cout << "Executando um deposito de " << amount << " para " << depositor << endl;
        error = deposit(bank_state, depositor, amount);
        break;
      }
      //Alterado
      case Action::Withdraw: {
        string withdrawer = nondet_picks["withdrawer"]["value"];
        int amount = int_from_json(nondet_picks["amount"]["value"]);
        cout << "Executando um saque de " << amount << " para " << withdrawer << endl;
        error = withdraw(bank_state, withdrawer, amount);
        break;
      }
      case Action::Transfer: {
        string sender = nondet_picks["sender"]["value"];
        string receiver = nondet_picks["receiver"]["value"];
        int amount = int_from_json(nondet_picks["amount"]["value"]);
        cout << "Executando transferência de " << amount << " de " << sender << " para " << receiver << endl;
        error = transfer(bank_state, sender, receiver, amount);
        break;
      }
      case Action::BuyInvestment: {
        string buyer = nondet_picks["buyer"]["value"];
        int amount = int_from_json(nondet_picks["amount"]["value"]);
        cout << "Executando compra de investimento no valor de " << amount << " pelo comprador " << buyer << endl;
        error = buy_investment(bank_state, buyer, amount);
        break;
      }
      case Action::SellInvestment: {
        string seller = nondet_picks["seller"]["value"];
        int id = int_from_json(nondet_picks["id"]["value"]);
        cout << "Executando venda do investimento de ID " << id << " pelo vendedor " << seller << endl;
        error = sell_investment(bank_state, seller, id);
        break;
      }  
      //fim da alteração
      default: {
        error = "";
        break;
      }
      }

      BankState expected_bank_state = bank_state_from_json(state["bank_state"]);

      cout <<"-----------------" << endl;
      cout <<"Resultado da implementação:" << endl;
      print_bank_state(bank_state);
      cout <<"Resultado do modelo (trace):" << endl;
      print_bank_state(expected_bank_state);
      cout <<"-----------------" << endl;
      

      //Saldos
      for (const auto& [account, expected_balance] : expected_bank_state.balances) {
        int real_balance = bank_state.balances[account];
        if (real_balance < expected_balance) {
            cout << "Ação: " << action << endl;
            cout << "Conta '" << account << "': era pra ter MAIS saldo. Esperado: "
                 << expected_balance << ", Obtido: " << real_balance << endl;
        } else if (real_balance > expected_balance) {
            cout << "Ação: " << action << endl;
            cout << "Conta '" << account << "': era pra ter MENOS saldo. Esperado: "
                 << expected_balance << ", Obtido: " << real_balance << endl;
        }
        assert(real_balance == expected_balance);
      }


      
      
    
      

      cout << "TODO: comparar o estado esperado com o estado obtido" << endl;
      
      string expected_error = string(state["error"]["tag"]).compare("Some") == 0
                                  ? state["error"]["value"]
                                  : "";



          
      if (error != expected_error) {
          cout << "Erro divergente na ação: " << action << endl;
          cout << "Esperado: \"" << expected_error << "\"" << endl;
          cout << "Obtido:   \"" << error << "\"" << endl;
      }
      assert(error == expected_error);
                                  
      cout << "TODO: comparar o erro esperado com o erro obtido" << endl;
    }
  }
  return 0;
}
