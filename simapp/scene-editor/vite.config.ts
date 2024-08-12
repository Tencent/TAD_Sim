import { fileURLToPath } from 'node:url'
import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import copy from 'rollup-plugin-copy'
import svgPlugin from 'vite-svg-loader'
import vueJsx from '@vitejs/plugin-vue-jsx'

// https://vitejs.dev/config/
export default defineConfig(({ command }) => {
  const config = {
    base: './',
    plugins: [
      vue(),
      svgPlugin({
        defaultImport: 'component',
      }),
      vueJsx({}),
    ],
    resolve: {
      alias: {
        '@': fileURLToPath(new URL('./src', import.meta.url)),
        'models-manager': fileURLToPath(new URL('./src/models-manager', import.meta.url)),
        'sim-player': fileURLToPath(new URL('./src/sim-player', import.meta.url)),
      },
    },
    server: {
      port: 8080,
      host: '0.0.0.0',
    },
    build: {
      outDir: 'build',
    },
  }

  if (command === 'serve') {
    config.plugins.push(
      copy({
        targets: [
          {
            src: 'src/models-manager/assets/*',
            dest: 'assets',
          },
        ],
      }),
    )
  } else if (command === 'build') {
    config.plugins.push(
      copy({
        targets: [
          {
            src: 'src/models-manager/assets/*',
            dest: 'build/assets',
          },
        ],
        hook: 'writeBundle',
      }),
    )
  }

  return config
})
