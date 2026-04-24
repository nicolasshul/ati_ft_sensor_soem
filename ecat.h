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

typedef struct {
  uint32 vendor_id;
  uint32 product_code;
  uint32 revision;
  uint32 serial_num;
} ati_identity_t;

typedef struct {
  char ft_serial[9];
  char cal_part[31];
  char cal_family[9];
  char cal_time[31];
  uint8_t force_units;
  uint8_t torque_units;
  int32_t counts_per_force;
  int32_t counts_per_torque;
} ati_calibration_t;

typedef struct {
  int32_t Rx;
  int32_t Ry;
  int32_t Rz;
  int32_t Dx;
  int32_t Dy;
  int32_t Dz;
} ati_transform_t;

typedef struct {
  ati_identity_t identity;
  ati_calibration_t calibration;
  ati_transform_t transform;
} ati_sdo_t;


#define NUMBER_OF_SLAVES 1
#define IOMAP_SIZE 4096

uint32 verify_network_configuration(ecx_contextt * ctx) {
  if (ctx->slavecount == NUMBER_OF_SLAVES) {
    return 0;
  }
  else {
    return 1;
  }
}

