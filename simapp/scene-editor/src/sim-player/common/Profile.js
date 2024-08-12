import EditorGlobals from '../sceneeditor/EditorGlobals'

/**
 * 外部配置信息
 */
const Profile = {
  operationMode: 1,
}

const signal = EditorGlobals.signals.operationMode
signal.add((mode) => {
  Profile.operationMode = mode
}, null, 10)

export default Profile
