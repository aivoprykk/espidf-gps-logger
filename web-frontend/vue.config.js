module.exports = {
  devServer: {
    port: 8082,
    proxy: {
      '/api': {
        target: 'http://esp.local:80',
        changeOrigin: true
      }
    }
  }
}
