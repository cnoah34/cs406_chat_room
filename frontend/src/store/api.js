import { defineStore } from 'pinia'

export const useApiStore = defineStore('api', {
    state: () => ({
        rest_url: 'http://localhost:8080',
    }),
    persist: true,
})
