#include <string.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <sstream>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>   

using namespace std;

typedef struct card_details {
	string name;
	int sale_price;
	int market_price;
	int profit;
	int ratio;
	int include;
	card_details(int i) : sale_price(i), market_price(i), profit(i), ratio(i), include(i){ }
} card;

//=======GLOBALS========\\
extern int max_profit;
extern int calc_max_profit;
extern int cost;
extern int total_weight;
extern int backtrack;
int max_profit = 0;
int calc_max_profit = 0;
int cost = 0;
int backtrack = 0;
int total_weight = 0;

void output_results(ofstream &file, int &input_num, vector<card> &cards, int &final_num, timeval &starttime, timeval &endtime);
vector<card> greedy1(ofstream &file, vector<card> &cards, int &cash, int print);
vector<card> greedy2(ofstream &file, vector<card> &cards, int &cash, int print);
bool ratio_descending(const card& card1, const card& card2){ return (card1.ratio > card2.ratio); }
int compare_prices(vector<card> &market_cards, vector<card> &available_cards, int index);
void knapsack(int index, int profit, int weight, vector<card> &cards, vector<card> &best_set, int &sub_avoid, int &sub_consider);
void backtracking(ofstream &file, vector<card> &cards);
bool promising(int index, int weight, int profit, vector<card> &cards);
int knapsack_fraction(int index, int weight, vector<card> &cards);

int main (int argc, char* argv[]) {
	int market_cards   = 0,
		spending_money = 0,
		element 	   = 0,
		input_size     = 0;
	string market_list = "",
		output_file	   = "",
		price_list     = "",
		algo_choice    = "",
		buff           = "";
	vector<card> cards, market_deck, trash;

	//command line: $> ./program3 –m <market-price-file> -p <price-list-file> -o <output-file-name> [0|1|2]
	if (argc != 8) cout << "Incorrect command line args." << endl;

	market_list = argv[2];
	price_list = argv[4];
	output_file = argv[6];
	algo_choice =argv[7];
	element = atoi(algo_choice.c_str());

	ifstream marketfile;
	marketfile.open(market_list.c_str());
	ifstream pricefile;
	pricefile.open(price_list.c_str());
	ofstream output;
	output.open(output_file.c_str());
	
	if(!(output.is_open())) {
		cout << "Error opening output file." << endl;
		return 1;
	}

	if(!(pricefile.is_open() || marketfile.is_open())) {
		cout << "File name provided does not exist." << endl;
		return 1;
	}
	int firstrun = 1;
	int indexcount = 0;
	int linecount = 0;
   	stringstream linestream;
	while (getline(marketfile, buff)) {
    	linestream.str(buff);
    	if (linecount == 0) {
    		linestream >> market_cards;
    		linestream.clear();
    	 	linecount++;
    	} else {
			card market_card(0);
	     	linestream >> market_card.name >> market_card.market_price;
	     	market_deck.push_back(market_card);
	     	linestream.clear();
	       	linecount++;
		}  
	}	

	//get all cards per problem. call either greedy1, greedy2, or backtrack.
	linecount = 0;
	while (getline(pricefile, buff)) {
    	linestream.str(buff);
    	if (linecount == 0) {
    		linestream >> input_size >> spending_money;
    		cost = spending_money;
    		linestream.clear();
    	 	linecount++;
    	} else {
	    	card baseball(0);
	     	linestream >> baseball.name >> baseball.sale_price;
	     	linestream.clear();
	    	cards.push_back(baseball);
	       	linecount++;
			if (linecount == input_size + 1) {
				if (!firstrun) indexcount+= input_size-1;
				if (compare_prices(market_deck, cards, indexcount)) break;
				if (element == 0){
					trash = greedy1(output, cards, spending_money, 1);
				}
				else if (element == 1){
					trash = greedy2(output, cards, spending_money, 1);
				}
				else {
					backtracking(output, cards);
				}
				linecount = 0;
				cards.clear();
				firstrun = 0;
			}
		}  
	}
	output.close();
	marketfile.close();
	pricefile.close();
	return 0;
}

//sort the cards by non-increasing [profit/weight] ratios. 
vector<card> greedy1(ofstream &file, vector<card> &cards, int &cash, int print) {
	int profit = 0;
	int cost1 = 0;
    vector<card> purchased;
	struct timeval starttime, endtime;
	gettimeofday(&starttime, 0);
	sort(cards.begin(), cards.end(), ratio_descending); 
	//finds a card that is affordable and adds it to purchased vector. If a card is not affordable, it is skipped and the next one is purchased
 	for (vector<card>::iterator it = cards.begin() ; it != cards.end(); ++it) {
 		int sell_prof = profit;
 		int tot_cost = cost1;
    	sell_prof += (*it).profit;
    	tot_cost += (*it).sale_price;
		if (tot_cost <= cash) {
			cost1 = tot_cost;
			profit = sell_prof;
			purchased.push_back(*it);
		}
	}	
	gettimeofday(&endtime, 0);
    int subsize = purchased.size();
    int total_cards = cards.size();
    card prof(0);
    prof.profit = profit;
    //profit is stored on back of vector of purchased cards
    purchased.push_back(prof);
	if (print) output_results(file, total_cards, purchased, subsize, starttime, endtime);
	return purchased;
}

//compare greedy1 with card that is affordable and has highest profit
vector<card> greedy2(ofstream &file, vector<card> &cards, int &cash, int print) {
	int pmax 	  = 0,
		maxprofit = 0,
		sell_prof = 0,
		subsize   = 0,
		print2    = 0;
    vector<card> purchased;
	struct timeval starttime, endtime;
    int total_cards = cards.size();
    card max_prof(0);
	gettimeofday(&starttime, 0);
	purchased = greedy1(file, cards, cash, print2);
    subsize = purchased.size() - 1;
	for (vector<card>::iterator it = cards.begin() ; it != cards.end(); ++it) {
		sell_prof = (*it).profit;
		if ((*it).sale_price <= cash) {
			if (pmax < sell_prof) {
				pmax = sell_prof;
				max_prof.name = (*it).name;
				max_prof.sale_price = (*it).sale_price;
				max_prof.market_price = (*it).market_price;
				max_prof.ratio = (*it).ratio;
				max_prof.profit = (*it).profit;
			}
		}
	}
	maxprofit = max( (purchased.back()).profit, pmax);
	gettimeofday(&endtime, 0);
	//if max profit is not equal to the profit generated by greedy1
	if (maxprofit > purchased.back().profit) {
		purchased.clear();
		if (print) {
			file << "Greedy 2: " << total_cards << " " << pmax << " " << 1 << " " << (float)(endtime.tv_sec  - starttime.tv_sec) << endl;
			file << (max_prof).name << endl;
		}
		purchased.push_back(max_prof);
	} else {
		//NOTE: prints output as if greedy1 was called if answer from greedy1 was used
		if (print) output_results(file, total_cards, purchased, subsize, starttime, endtime);
	}
	return purchased;
}

void backtracking(ofstream &file, vector<card> &cards) {
	max_profit = 0;
	total_weight = 0;
	
	int sub_avoid 	 = 0,
		sub_consider = 0,
		print 		 = 0;
    
    int total_cards = cards.size();
	vector<card> best_set;
	int cash = cost;
	//gets best element or set of elements returned from greedy2
	best_set = (greedy2(file, cards, cash, print));	
	//creates place holder for first index of vector
	card first_array_holder(0);
	first_array_holder.name = "root";
	cards.insert (cards.begin(), first_array_holder);
	//global calc_max_profit is greated profit generated from greedy2
	calc_max_profit = (best_set.back()).profit;
	//greedy2 returns a set of cards with the total profit stored in the final card. This needs to be removed
	best_set.pop_back();
	int num_set = best_set.size() - 1;
	
	// timeval starttime, endtime;
	struct timeval starttime, endtime;
   	gettimeofday( &starttime, NULL );

	//knapsack called with initial values for profit, weight, and index = 0
	knapsack(0, cards.at(0).profit, cards.at(0).sale_price, cards, best_set, sub_avoid, sub_consider);
	card max_prof(0);
	max_prof.profit = max_profit;
	//max_profit is a global. Once knapsack completes, the max_profit is stored on the end of the best set array
	best_set.push_back(max_prof);
	gettimeofday(&endtime, 0);

	file << "Backtracking: ";
	file << total_cards << " " << (best_set.back()).profit << " " << num_set << " " <<  (float)(endtime.tv_sec  - starttime.tv_sec) << endl;
	file << sub_consider << " subsets considered : " << sub_avoid << " subsets avoided via backtracking" << endl;
	for (vector<card>::iterator it = best_set.begin() ; it != best_set.end() - 1; ++it) {
		file << (*it).name << endl;
	}
}



void knapsack(int index, int profit, int weight, vector<card> &cards, vector<card> &best_set, int &sub_avoid, int &sub_consider) {
	if (weight <= cost && profit > max_profit) {
		max_profit = profit;
		if (max_profit > calc_max_profit) {
			best_set.clear();
			for (vector<card>::iterator it = cards.begin() + 1; it != cards.end(); ++it) {
				if ( (*it).include == 1 ) {
					best_set.push_back(*it);
				}	
			}
		}
	}
	sub_consider++;
	total_weight = weight;
	if (promising(index, weight, profit, cards)) {
		total_weight = 0;
		if (index + 1 < cards.size()) {
			cards.at(index + 1).include = 1;
			knapsack(index + 1, profit + cards.at(index + 1).profit, weight + cards.at(index + 1).sale_price, cards, best_set, sub_avoid, sub_consider);
			cards.at(index + 1).include = 0;
			knapsack(index + 1, profit, weight, cards, best_set, sub_avoid, sub_consider);
		}
	} else {
		sub_avoid += pow(2, cards.size() - 1 - index);
	}
}


bool promising(int index, int weight, int profit, vector<card> &cards) {
	int known_profits = profit;
    if (weight >= cost) {
    	return false; 
    }
    int bound = knapsack_fraction(index, weight, cards) + profit;
    if (bound < max_profit) return false;
    return (bound > calc_max_profit);
}

int knapsack_fraction(int index, int known_weight, vector<card> &cards) {
	int n = cards.size() - 1;
	float bound = 0;
	int weight = known_weight;
	while ((weight < cost) && (index < n)) {
		if ( (weight + cards.at(index+1).sale_price) <= cost) {              
			weight += cards.at(index+1).sale_price; 
			bound += cards.at(index+1).profit;
		}
		else {
        	float fraction = (float)(cost - weight)/ (float)cards.at(index+1).sale_price;
        	weight = cost;
        	bound += cards.at(index+1).profit * fraction; 
    	}
		index = index + 1;
	}
	return bound;
}

void output_results(ofstream &file, int &input_num, vector<card> &cards, int &final_num, timeval &starttime, timeval &endtime) {
	file << "Greedy 1: "  << input_num << " " << (cards.back()).profit << " " << final_num << " " <<  (float)(endtime.tv_sec  - starttime.tv_sec) << endl;
	for (vector<card>::iterator it = cards.begin() ; it != cards.end() - 1; ++it) {
		file << (*it).name << endl;
	}
}

//matches the cards in the price list with cards in the market list.
//this used to be 2 for loops, but this wasn't feasible for the million element lists.
int compare_prices(vector<card> &market_cards, vector<card> &cards, int index) {
	int i = 0;
	while (i < cards.size()) {
		if (cards.at(i).name == (market_cards.at(index)).name ) {
			(cards.at(i)).market_price = (market_cards.at(index)).market_price;
			(cards.at(i)).profit = ( (market_cards.at(index)).market_price - (cards.at(i)).sale_price );
			(cards.at(i)).ratio = (cards.at(i)).profit / (cards.at(i)).sale_price;
			index++;
			i++;
		} else {
			cout << "Error: " << (cards.at(i)).name << " not found in market file. " << endl;
			return 1;	
		}
	}
	return 0;
}

