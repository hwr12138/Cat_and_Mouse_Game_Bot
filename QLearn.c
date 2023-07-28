#include "QLearn.h"

void QLearn_update(int s, int a, double r, int s_new, double *QTable)
{
 /*
   This function implementes the Q-Learning update as stated in Lecture. It 
   receives as input a <s,a,r,s'> tuple, and updates the Q-table accordingly.
   
   Your work here is to calculate the required update for the Q-table entry
   for state s, and apply it to the Q-table
     
   The update involves two constants, alpha and lambda, which are defined in QLearn.h - you should not 
   have to change their values. Use them as they are.
     
   Details on how states are used for indexing into the QTable are shown
   below, in the comments for QLearn_action. Be sure to read those as well!
 */
    // s is the current state, a is the action taken, r is the reward obtained,
    // s_new is the new state, and QTable is the Q-Table
    
    // Get the current Q-Value for the (s,a) pair
    double Qsa = *(QTable + (4 * s) + a);
    
    // Calculate the maximum Q-Value for the new state
    double Qmax = *(QTable + (4 * s_new) + 0);
    for (int i = 1; i < 4; i++) {
        double Qnew = *(QTable + (4 * s_new) + i);
        if (Qnew > Qmax) {
            Qmax = Qnew;
        }
    }
    
    // Calculate the new Q-Value for the (s,a) pair
    double Qnew = Qsa + alpha * (r - Qsa); // 
    
    // Update the Q-Table
    *(QTable + (4 * s) + a) = Qnew;
  
}

int QLearn_action(double gr[max_graph_size][4], int mouse_pos[1][2], int cats[5][2], int cheeses[5][2], double pct, double *QTable, int size_X, int graph_size)
{
  /*
     This function decides the action the mouse will take. It receives as inputs
     - The graph - so you can check for walls! The mouse must never move through walls
     - The mouse position
     - The cat position
     - The chees position
     - A 'pct' value in [0,1] indicating the amount of time the mouse uses the QTable to decide its action,
       for example, if pct=.25, then 25% of the time the mouse uses the QTable to choose its action,
       the remaining 75% of the time it chooses randomly among the available actions.
   */
  // get the current state of the game
    int i = mouse_pos[0][0];
    int j = mouse_pos[0][1];
    int k = cats[0][0];
    int l = cats[0][1];
    int m = cheeses[0][0];
    int n = cheeses[0][1];
    int state = (i + j * size_X) + ((k + l * size_X) * graph_size) + ((m + n * size_X) * graph_size * graph_size);
    int index = i + (j * size_X);
    // use the Q-table pct percent of the time, choose randomly otherwise
    int action;
    double c = (double)rand() / (double)((unsigned)RAND_MAX + 1);
    
    if (c <= pct) {
        // choose action with the highest Q-value for the current state
        double maxQ = *(QTable + (4 * state));
        action = 0;
        for (int a = 1; a < 4; a++) {
            double Qval = *(QTable + (4 * state) + a);
            if ((Qval > maxQ) && (gr[index][a] != 0) || (gr[index][action] == 0)) {
                maxQ = Qval;
                action = a;
            }
        }
    } else {
        // choose a random action
        int possible_actions[4];
        int num_actions = 0;
        for (int a = 0; a < 4; a++) {
            if (gr[index][a] != 0) {
                possible_actions[num_actions] = a;
                num_actions++;
            }
        }
        int rand_idx = rand() % num_actions;
        action = possible_actions[rand_idx];
    }

    // update mouse position based on the chosen action
    int new_i = i;
    int new_j = j;
    if (action == 0 && gr[index][0] != 0) {
        new_j = j - 1;
    } else if (action == 1 && gr[index][1] != 0) {
        new_i = i + 1;
    } else if (action == 2 && gr[index][2] != 0) {
        new_j = j + 1;
    } else if (action == 3 && gr[index][3] != 0) {
        new_i = i - 1;
    }

    // check if the new position is valid (i.e., not a wall)
    if (gr[index][action] == 0) {
        printf("Warning: mouse crossed a wall! Action: %d, index: %d\n", action, index);
        printf("QTable val: %f\n", *(QTable + (4 * state) + action));
    } else if (new_i < 0 || new_i >= size_X || new_j < 0 || new_j >= size_X) {
        printf("Warning: mouse left the map! Action: %d, index: %d\n", action, index);
        printf("QTable val: %f\n", *(QTable + (4 * state) + action));
    }

    int new_s = (new_i + new_j * size_X) + ((k + l * size_X) * graph_size) + ((m + n * size_X) * graph_size * graph_size);
    int new_mouse_pos[1][2];
    new_mouse_pos[0][0] = new_i;
    new_mouse_pos[0][1] = new_j;
    
    double reward = QLearn_reward(gr, new_mouse_pos, cats, cheeses, size_X, graph_size);
    QLearn_update(state, action, reward, new_s, QTable);
    
    return action;
}

double QLearn_reward(double gr[max_graph_size][4], int mouse_pos[1][2], int cats[5][2], int cheeses[5][2], int size_X, int graph_size)
{
  /*
    This function computes and returns a reward for the state represented by the input mouse, cat, and
    cheese position.    
   */
  // converges to 0.85 for 50 rounds
  // take number of moves into account
  int index = mouse_pos[0][0] + mouse_pos[0][1]*size_X;
  int possible_moves = 0;
  for (int i = 0; i < 4; i++){
    if (gr[index][i] != 0){
      possible_moves++;
    }
  }
  //take into account how "center" is the mouse
  int dist_to_center = abs(mouse_pos[0][0] - (size_X/2)) + abs(mouse_pos[0][1] - (size_X/2));
  // take closest cat distance into account
  int min_cat_dist = 10000;
  for(int i = 0; i < 1; i++){
    int mouse_dist = abs(cats[i][0]-mouse_pos[0][0]) + abs(cats[i][1]-mouse_pos[0][1]);
    if (mouse_dist < min_cat_dist){
      min_cat_dist = mouse_dist;
    }
  }
  if (min_cat_dist == 0){
      min_cat_dist -= size_X * size_X * size_X; 
  }
  if (min_cat_dist <= 1){
      min_cat_dist -= size_X; 
  }
  //take into account closest cheese 
  int min_cheese_dist = 1000;
  for(int i = 0; i < 1; i++){
    int mouse_dist = abs(cheeses[i][0]-mouse_pos[0][0]) + abs(cheeses[i][1]-mouse_pos[0][1]);
    if (mouse_dist < min_cheese_dist){
      min_cheese_dist = mouse_dist;
    }
  }
  
  int cat_to_cheese = abs(cats[0][0] - cheeses[0][0]) + abs(cats[0][1] - cheeses[0][1]);
  if (min_cheese_dist == 0){min_cheese_dist -= size_X*size_X;}
  if (min_cheese_dist <= 1){min_cheese_dist -= 2*size_X;}
  int reward = - min_cheese_dist + min_cat_dist + possible_moves - 0.2*dist_to_center;
  //printf("reward: %d, %d, %d, %d\n", min_cheese_dist, min_cat_dist, possible_moves, dist_to_center);
  return(reward);
}


void feat_QLearn_update(double gr[max_graph_size][4],double weights[25], double reward, int mouse_pos[1][2], int cats[5][2], int cheeses[5][2], int size_X, int graph_size)
{
  /*
    This function performs the Q-learning adjustment to all the weights associated with your
    features. Unlike standard Q-learning, you don't receive a <s,a,r,s'> tuple, instead,
    you receive the current state (mouse, cats, and cheese potisions), and the reward 
    associated with this action (this is called immediately after the mouse makes a move,
    so implicit in this is the mouse having selected some action)
   */
  double feat_orig[25], feat_after[25], maxU;
  int new_pos[1][2], maxA;
  evaluateFeatures(gr, feat_orig, mouse_pos, cats, cheeses, size_X, graph_size);
  maxQsa(gr, weights, mouse_pos, cats, cheeses, size_X, graph_size, &maxU, &maxA);
  position(mouse_pos, new_pos, maxA);
  evaluateFeatures(gr, feat_after, new_pos, cats, cheeses, size_X, graph_size);
  for (int i = 0; i < numFeatures; i++) {
    double qs = Qsa(weights, feat_orig);
    double qs_aft = Qsa(weights, feat_after);
    double res = (alpha * (reward + (lambda * qs_aft) - qs) * feat_orig[i]);
    weights[i] += res;
  }
}

int feat_QLearn_action(double gr[max_graph_size][4],double weights[25], int mouse_pos[1][2], int cats[5][2], int cheeses[5][2], double pct, int size_X, int graph_size)
{
  /*
    Similar to its counterpart for standard Q-learning, this function returns the index of the next
    action to be taken by the mouse.
    
    Once more, the 'pct' value controls the percent of time that the function chooses an optimal
    action given the current policy.
    
    E.g. if 'pct' is .15, then 15% of the time the function uses the current weights and chooses
    the optimal action. The remaining 85% of the time, a random action is chosen.  
   */
  double c = ((double) rand()) / (double)((unsigned) RAND_MAX + 1);
  int curr_index = mouse_pos[0][0] + (mouse_pos[0][1] * size_X);
  int a;
  double maxU;
  int abc=0;
  if (c > pct) {
    a = rand()%4;
    while (gr[curr_index][a] == 0) {
      a = rand()%4;
    }
    abc = 1;
  } else {
    // optimal action
    maxQsa(gr, weights, mouse_pos, cats, cheeses, size_X, graph_size, &maxU, &a);
    abc = 2;
  }
  return a;
}

void evaluateFeatures(double gr[max_graph_size][4],double features[25], int mouse_pos[1][2], int cats[5][2], int cheeses[5][2], int size_X, int graph_size)
{
  /*
   This function evaluates all the features you defined for the game configuration given by the input
   mouse, cats, and cheese positions. 
  */
  int cat_index=0, chee_index = 0;
  int curr_index = mouse_pos[0][0] + (mouse_pos[0][1] * size_X);
  double count;
  for (int i = 0; i < 5; i++) {
    if (cats[i][0] != -1) {
      double d = (double)(abs(mouse_pos[0][0] - cats[i][0]) + abs(mouse_pos[0][1] - cats[i][1])) / size_X / 2;
      if (i == 0) {
        count = d;
        cat_index = i;
      } else if (d < count){
        count = d;
        cat_index = i;
      }
    }
  }
  // min cat dist
  features[0] = count;
  for (int i = 0; i < 5; i++) {
    if (cheeses[i][0] != -1) {
      double d = (double)(abs(mouse_pos[0][0] - cheeses[i][0]) + abs(mouse_pos[0][1] - cheeses[i][1])) / size_X / 2;
      if (i == 0) {
        count = d;
        chee_index = i;
      } else if (d < count){
        count = d;
        chee_index = i;
      }
    }
  }
  // min cheese dist
  features[1] = count;
  int ind = 2;
  features[ind] = 0;
  if (mouse_pos[0][0] - cats[cat_index][0] < 0 && gr[curr_index][1] == 0) {
    features[ind] = 1;
  } else if (mouse_pos[0][0] - cats[cat_index][0] > 0 && gr[curr_index][3] == 0) {
    features[ind] = 1;
  } else if (mouse_pos[0][1] - cats[cat_index][1] < 0 && gr[curr_index][2] == 0) {
    features[ind] = 1;
  } else if (mouse_pos[0][1] - cats[cat_index][1] > 0 && gr[curr_index][0] == 0) {
    features[ind] = 1;
  }
  ind = 3;
  features[ind] = 0;
  if (mouse_pos[0][0] - cheeses[chee_index][0] < 0 && gr[curr_index][1] == 0) {
    features[ind] = 1;
  } else if (mouse_pos[0][0] - cheeses[chee_index][0] > 0 && gr[curr_index][3] == 0) {
    features[ind] = 1;
  } else if (mouse_pos[0][1] - cheeses[chee_index][1] < 0 && gr[curr_index][2] == 0) {
    features[ind] = 1;
  } else if (mouse_pos[0][1] - cheeses[chee_index][1] > 0 && gr[curr_index][0] == 0) {
    features[ind] = 1;
  }
  double dist[5];
  // cat dist descending
  for (int i = 0; i < 5; i++) {
    if (cats[i][0] != -1) {
      double d = (double)(abs(mouse_pos[0][0] - cats[i][0]) + abs(mouse_pos[0][1] - cats[i][1])) / size_X / 2;
      if (i == 0) {
        dist[0] = d;
      } else {
        // insert
        double temp = d, temp2;
        for (int j = 0; j < i; j++) {
          if (temp > dist[j]) {
            temp2 = dist[j];
            dist[j] = temp;
            temp = dist[j+1];
            dist[j+1] = temp2;
          }
        }
      }
    } else {
      dist[i] = 0;
    }
  }
  for (int i = 0; i < 5; i++) {
    features[i+4] = dist[i];
  }
  // cheese dist desc
  for (int i = 0; i < 5; i++) {
    if (cheeses[i][0] != -1) {
      double d = (double)(abs(mouse_pos[0][0] - cheeses[i][0]) + abs(mouse_pos[0][1] - cheeses[i][1])) / size_X / 2;
      if (i == 0) {
        dist[0] = d;
      } else {
        // insert
        double temp = d, temp2;
        for (int j = 0; j < i; j++) {
          if (temp > dist[j]) {
            temp2 = dist[j];
            dist[j] = temp;
            temp = dist[j+1];
            dist[j+1] = temp2;
          }
        }
      }
    } else {
      dist[i] = 0;
    }
  }
  for (int i = 0; i < 5; i++) {
    features[i+9] = dist[i];
  }
}

double Qsa(double weights[25], double features[25])
{
  /*
    Compute and return the Qsa value given the input features and current weights
   */
  double result = 0;
  for (int i = 0; i < numFeatures; i++) {
    result += weights[i] * features[i];
  }
  return result;
}

void maxQsa(double gr[max_graph_size][4],double weights[25],int mouse_pos[1][2], int cats[5][2], int cheeses[5][2], int size_X, int graph_size, double *maxU, int *maxA)
{
 /*
   Given the state represented by the input positions for mouse, cats, and cheese, this function evaluates
   the Q-value at all possible neighbour states and returns the max. The maximum value is returned in maxU
   and the index of the action corresponding to this value is returned in maxA.
  */
  double features[25];
  int curr_index = mouse_pos[0][0] + (mouse_pos[0][1] * size_X);
  double max;
  int init = 0;
  int new_pos[1][2];
  for (int k = 0; k < 4; k++) {
    if (gr[curr_index][k] == 0) {
      continue;
    }
    position(mouse_pos, new_pos, k);
    evaluateFeatures(gr, features, new_pos, cats, cheeses, size_X, graph_size);
    double val = Qsa(weights, features);
    if (init == 0) {
      max = val;
      *maxA = k;
      *maxU = val;
      init = 1;
    }
    else if (val > max) {
      max = val;
      *maxA = k;
      *maxU = val;
    }
  }
  return;
}

void position(int mouse_pos[1][2], int new_pos[1][2], int a) {
  if (a == 0) {
    new_pos[0][0] = mouse_pos[0][0];
    new_pos[0][1] = mouse_pos[0][1] - 1;
  } else if (a == 1) {
    new_pos[0][0] = mouse_pos[0][0] + 1;
    new_pos[0][1] = mouse_pos[0][1];
  } else if (a == 2) {
    new_pos[0][0] = mouse_pos[0][0];
    new_pos[0][1] = mouse_pos[0][1] + 1;
  } else if (a == 3) {
    new_pos[0][0] = mouse_pos[0][0] - 1;
    new_pos[0][1] = mouse_pos[0][1];
  }
}
