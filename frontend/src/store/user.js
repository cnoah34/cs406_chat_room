import { defineStore } from 'pinia'

export const useUserStore = defineStore('user', {
    state: () => ({
        username: null,
        created_at: null,
        room_ids: []
    }),
    actions: {
        setUser(data) {
            this.username = data.username
            this.created_at = data.created_at
            this.room_ids = data.room_ids
        },
        clearUser() {
            this.$reset()
            localStorage.removeItem('user')
        }
    },
    persist: true,
})
