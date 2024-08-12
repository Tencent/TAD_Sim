import type { CatalogCategory, CatalogModel } from 'models-manager/src/catalogs/class.ts'
import ajax from '@/api/axios'

// const baseUrl = 'http://127.0.0.1:9000/api'

interface UploadResult {
  id: string
  filepath: string[]
}

export function uploadModelZip (path: string): Promise<UploadResult> {
  return ajax.post('http://127.0.0.1:9000/scenemodel/upload', { model_path: path })
}

export function uploadThumbnail (file: File, dir: string): Promise<{ dstpath: string }> {
  const formData = new FormData()
  formData.append('file', file)
  formData.append('model_dir', dir)
  return ajax.post('http://127.0.0.1:9000/scenemodel/upload_thumbnail', formData, {
    headers: {
      'Content-Type': 'multipart/form-data',
    },
  })
}

export function saveModel (id: string, model: CatalogModel): Promise<{ code: number, message: string }> {
  return ajax.post('http://127.0.0.1:9000/scenemodel/save', { id, data: model })
}

export function delModel (
  variable: string,
  catalogCategory: CatalogCategory,
): Promise<{ code: number, message: string }> {
  return ajax.post('http://127.0.0.1:9000/scenemodel/delete', { variable, catalogCategory })
}
