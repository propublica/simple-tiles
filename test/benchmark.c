#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <simple-tiles/simple_tiles.h>


static void*
setup_map(){
  simplet_map_t *map;
  assert((map = simplet_map_new()));
  return map;
}

static void
teardown_map(void *ctx){ 
  simplet_map_free(ctx);
}


static void*
setup_list(){
  simplet_list_t *list;
  assert((list = simplet_list_new()));
  return list;
}

static void
teardown_list(void *ctx){
  simplet_list_free(ctx);
}

static void
initialize_map(simplet_map_t *map){
  simplet_map_set_size(map, 256, 256);
  simplet_map_set_slippy(map, 0, 1, 2);
  simplet_map_add_layer(map, "../data/10m_admin_0_countries.shp");
  simplet_map_add_filter(map,  "SELECT * from '10m_admin_0_countries'");
  simplet_map_add_style(map, "weight", "0.1");
  simplet_map_add_style(map, "fill",   "#061F37ff");
}

cairo_status_t
stream(void *closure, const unsigned char *data, unsigned int length){
  return CAIRO_STATUS_SUCCESS;
  data = NULL, length = 0, closure = NULL; /* suppress warnings */
}

static void
bench_seamless(void *ctx){
  simplet_map_t *map = ctx;
  initialize_map(map);
  simplet_map_add_style(map, "seamless", "true");
  char *data = NULL;
  assert(simplet_map_render_to_stream(map, data, stream));
}

static void
bench_render(void *ctx){
  simplet_map_t *map = ctx;
  initialize_map(map);
  char *data = NULL;
  assert(simplet_map_render_to_stream(map, data, stream));
}

#define ITEMS 1000000

static void
bench_list(void *ctx){
  simplet_list_t *list = ctx;
  for(int i = 0; i < ITEMS; i++)
    simplet_list_push(list, NULL);
}

typedef struct {
  const char *name;
  void *(*setup)();
  void (*call)(void *ctx);
  void (*teardown)(void *ctx);
  const int times;
} bench_wrap_t;

#define BENCH(around, name, times) \
  { #name, &setup_##around, &bench_##name, &teardown_##around, times },

bench_wrap_t benchmarks[] = {
  BENCH(map, render, 10)
  BENCH(map, seamless, 10)
  BENCH(list, list, 10)
  { NULL, NULL, NULL, NULL, 0 }
};

static double
sum(double *arr, int count){
  double sum = 0;
  for(int i = 0; i < count; i++)
    sum += arr[i];
  return sum;
}

static double
mean(double *arr, int count){ 
  return sum(arr, count) / count;
}

static double
stdev(double *arr, int count){
  double avg = mean(arr, count);
  double var = 0;
  for(int i = 0; i < count; i++)
    var += var + pow(arr[i] - avg, 2);
  return sqrt(var / count);
}

int
main(){
  bench_wrap_t *bench = (bench_wrap_t*)&benchmarks;
  for(bench = (bench_wrap_t*)&benchmarks; bench->call; bench++){
    printf("bench %s:\n", bench->name);
    double runs[bench->times];
    for(int i = 0; i < bench->times; i++){
      void *data = bench->setup();
      clock_t start = clock();
      bench->call(data);
      runs[i] = (double) (clock() - start) / CLOCKS_PER_SEC;
      printf("\x1b[1;32m.\x1b[0m");
      fflush(stdout);
      bench->teardown(data);
    }
    printf("\n\x1b[33mmean\x1b[0m: %f\n", mean(runs, bench->times));
    printf("\x1b[33mstd\x1b[0m:  %f\n",  stdev(runs, bench->times));
  }
}
