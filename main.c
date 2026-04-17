#include <stdio.h>

#include "soem/soem.h"

typedef struct __attribute__((packed)) {
  uint32_t control1;   // 0x7010:01
  uint32_t control2;   // 0x7010:02
} ati_rxpdo_t;

typedef struct __attribute__((packed)) {
  int32_t fx;          // 0x6000:01
  int32_t fy;          // 0x6000:02
  int32_t fz;          // 0x6000:03
  int32_t tx;          // 0x6000:04
  int32_t ty;          // 0x6000:05
  int32_t tz;          // 0x6000:06
} ati_txpdo_t;

#define NUMBER_OF_SLAVES 1
#define IOMAP_SIZE 4096

static ecx_contextt ctx;

uint32 verify_network_configuration(ecx_contextt * ctx)
{
  if (ctx->slavecount == NUMBER_OF_SLAVES) {
    return 0;
  }
  else {
    return 1;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage : %s <ifacename>\n", argv[0]);
    return 1;
  }

  char * iface = argv[1];

  // Sets up NIC
  if (ecx_int(&ctx, iface) <= 0) {
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
  int expectedWKC = ctx.group[0].outputsWKC * 2 + ctx.group[0].inputsWKC;

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
    fprintf(stderr, "Slave %d outputs pointer is NULL\n", slave);
    return 1;
  }

  if (in == NULL) {
    fprintf(stderr, "Slave %d inputs pointer is NULL\n", slave);
    return 1;
  }

  out->control1 = 0;
  out->control2 = 0;


  // Checking all slaves could enter operational mode
  chk = 40;
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

  // Operational state reached, starting 

  while (1) {
    // Keep control words zero unless command needs to be sent
    out->control1 = 0;
    out->control2 = 0;

    // If not continually sending process data watchdog will be sent
    ecx_send_processdata(&ctx);
    wkc = ecx_receive_processdata(&ctx, EC_TIMEOUTRET);

    if (wkc >= expectedWKC) {
      printf("Fx=%d  Fy=%d  Fz=%d  Tx=%d  Ty=%d  Tz=%d\n",
          in->fx, in->fy, in->fz,
          in->tx, in->ty, in->tz);
    }
    else {
      fprintf(stderr, "Bad WKC: got %d expected >= %d\n", wkc, expectedWKC);

      ecx_readstate(&ctx);
      for (int i = 1; i <= *(ctx.slavecount); i++) {
        if (ctx.slavelist[i].state != EC_STATE_OPERATIONAL) {
          fprintf(stderr,
              "Slave %d not OP. state = 0x%02x ALstatus = 0x%04x\n",
              i,
              ctx.slavelist[i].state,
              ctx.slavelist[i].ALstatuscode);
        }
      }
    }

    usleep(1000);
  }


}





}

