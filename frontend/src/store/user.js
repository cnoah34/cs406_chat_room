import { defineStore } from 'pinia'
import { ref } from 'vue'

export const useUserStore = defineStore('user', () => {
    const username = ref(null)
    const created_at = ref(null)
    const room_ids = ref([])

    function setUser(data) {
        username.value = data.username
        created_at.value = data.created_at
        room_ids.value = data.room_ids
    }

    function clearUser() {
        username.value = null
        created_at.value = null
        room_ids.value = []
    }

    return { username, created_at, room_ids, setUser, clearUser }
})
