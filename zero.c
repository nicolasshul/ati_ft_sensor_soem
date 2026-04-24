#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "soem/soem.h"
#include "ecat.h"


static ecx_contextt ctx;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage : %s <ifacename>\n", argv[0]);
    return 1;
  }

  char * iface = argv[1];

  // Sets up NIC
  if (ecx_init(&ctx, iface) <= 0) {
    fprintf(stderr, "Error: ecx_init\n");
    return 1;
  }

  // Configures the network
  if (ecx_config_init(&ctx) <= 0) {
    fprintf(stderr, "Error: ecx_config_init\n");
    return 1;
  }

  // Checks if the slavecount is equal to the expected amount
  if (verify_network_configuration(&ctx) == 1) {
    fprintf(stderr, "Error: verify_network_configuration\n");
  }

  // Maps the slaves to the IOmap
  uint8_t IOmap[IOMAP_SIZE];
  int size = ecx_config_map_group(&ctx, IOmap, 0);
  if (size > IOMAP_SIZE) {
    fprintf(stderr, "Error: iomap size too small\n");
  }

  // Calculates working counter
  int expectedWKC = ctx.grouplist[0].outputsWKC * 2 + ctx.grouplist[0].inputsWKC;

  // Configures EtherCat clock and measures propogation delay of slaves
  ecx_configdc(&ctx);

  // Requests slaves to enter safe op mode
  ecx_statecheck(&ctx, 0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);

  // Ensuring that all slaves have valid outputs
  ecx_send_processdata(&ctx);
  ecx_receive_processdata(&ctx, EC_TIMEOUTRET);

  // Requesting operational state for all slaves
  ctx.slavelist[0].state = EC_STATE_OPERATIONAL;
  ecx_writestate(&ctx, 0);

  // Setting up output and input structs
  ati_rxpdo_t *out = (ati_rxpdo_t *) ctx.slavelist[1].outputs;
  ati_txpdo_t *in  = (ati_txpdo_t *) ctx.slavelist[1].inputs;

  if (out == NULL) {
    fprintf(stderr, "Slave 1 outputs pointer is NULL\n");
    return 1;
  }

  if (in == NULL) {
    fprintf(stderr, "Slave 1 inputs pointer is NULL\n");
    return 1;
  }

  out->control1 = 0;
  out->control2 = 0;


  // Checking all slaves could enter operational mode
  int chk = 40;
  do {
    ecx_send_processdata(&ctx);
    ecx_receive_processdata(&ctx, EC_TIMEOUTRET);
    ecx_statecheck(&ctx, 0, EC_STATE_OPERATIONAL, 50000);
  } while (chk-- && (ctx.slavelist[0].state != EC_STATE_OPERATIONAL));
  if (ctx.slavelist[0].state != EC_STATE_OPERATIONAL) {
    fprintf(stderr, "Failed to reach OP state. Master state = 0x%02x\n",
        ctx.slavelist[0].state);
    return 1;
  }

  printf("In OP State, zeroing sensor...\n");
  int32_t ctrl = 0;
  int size = sizeof(ctrl);
  uint16_t slave = 1;
  // read current control word
  if (ecx_SDOread(ctx, slave, 0x7010, 0x01, FALSE, &size, &ctrl, EC_TIMEOUTRXM) <= 0) {
    printf("SDO read failed\n");
    return 1;
  }

  // set bit 0
  int32_t set_cmd = ctrl | (1 << 0);
  ecx_SDOwrite(ctx, slave, 0x7010, 0x01, FALSE, sizeof(set_cmd), &set_cmd, EC_TIMEOUTRXM);

  usleep(10000);

  int32_t clear_cmd = ctrl & ~(1 << 0);
  ec_SDOwrite(SLAVE, 0x7010, 0x01, FALSE, sizeof(clear_cmd), &clear_cmd, EC_TIMEOUTRXM);

  return 0;
}
