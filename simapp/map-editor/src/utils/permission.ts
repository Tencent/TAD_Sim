// 获取 configKey 对应的权限值
export function getPermission (configKey: string) {
  return (window as any).electron.getPermission(configKey)
}
