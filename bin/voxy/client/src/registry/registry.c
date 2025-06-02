#include <voxy/client/registry/name.h>

#include <assert.h>
#include <stb_ds.h>

static struct voxy_name_info *name_infos;

voxy_name_id_t voxy_register_name(struct voxy_name_info name_info)
{
  const voxy_name_id_t id = arrlenu(name_infos);
  arrput(name_infos, name_info);
  return id;
}

struct voxy_name_info voxy_query_name(voxy_name_id_t id)
{
  assert(id < arrlenu(name_infos));
  return name_infos[id];
}

const struct voxy_name_info *voxy_query_name_all(void)
{
  return name_infos;
}
