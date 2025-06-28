stl_fix_normal_directions(stl_file *stl) {
  char *norm_sw;
  int checked = 0;
  int facet_num;
  int i;
  int j;
  struct stl_normal {
    int               facet_num;
    struct stl_normal *next;
  };
  struct stl_normal *head;
  struct stl_normal *tail;
  struct stl_normal *newn;
  struct stl_normal *temp;
  if (stl->error) return;
  head = (struct stl_normal*)malloc(sizeof(struct stl_normal));
  if(head == NULL) perror("stl_fix_normal_directions");
  tail = (struct stl_normal*)malloc(sizeof(struct stl_normal));
  if(tail == NULL) perror("stl_fix_normal_directions");
  head->next = tail;
  tail->next = tail;
  norm_sw = (char*)calloc(stl->stats.number_of_facets, sizeof(char));
  if(norm_sw == NULL) perror("stl_fix_normal_directions");
  facet_num = 0;
  if(stl_check_normal_vector(stl, 0, 0) == 2)
    stl_reverse_facet(stl, 0);
  norm_sw[facet_num] = 1;
  checked++;
  for(;;) {
    for(j = 0; j < 3; j++) {
      if(stl->neighbors_start[facet_num].which_vertex_not[j] > 2) {
        if(stl->neighbors_start[facet_num].neighbor[j] != -1) {
          stl_reverse_facet
          (stl, stl->neighbors_start[facet_num].neighbor[j]);
        }
      }
      if(stl->neighbors_start[facet_num].neighbor[j] != -1) {
        if(norm_sw[stl->neighbors_start[facet_num].neighbor[j]] != 1) {
          newn = (struct stl_normal*)malloc(sizeof(struct stl_normal));
          if(newn == NULL) perror("stl_fix_normal_directions");
          newn->facet_num = stl->neighbors_start[facet_num].neighbor[j];
          newn->next = head->next;
          head->next = newn;
        }
      }
    }
    if(head->next != tail) {
      facet_num = head->next->facet_num;
      if(norm_sw[facet_num] != 1) {  
        norm_sw[facet_num] = 1;  
        checked++;
      }
      temp = head->next;	 
      head->next = head->next->next;
      free(temp);
    } else {  
      stl->stats.number_of_parts += 1;
      if(checked >= stl->stats.number_of_facets) {
        break;
      } else {
        for(i = 0; i < stl->stats.number_of_facets; i++) {
          if(norm_sw[i] == 0) {
            facet_num = i;
            if(stl_check_normal_vector(stl, i, 0) == 2) {
              stl_reverse_facet(stl, i);
            }
            norm_sw[facet_num] = 1;
            checked++;
            break;
          }
        }
      }
    }
  }
  free(head);
  free(tail);
  free(norm_sw);
}