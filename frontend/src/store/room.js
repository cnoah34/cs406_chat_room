import { defineStore } from 'pinia'
import { ref } from 'vue'

export const useRoomStore = defineStore('room', () => {
    const id = ref(null)
    const name = ref(null)
    const created_at = ref(null)

    function setRoom(data) {
        room_id.value = data.id
        name.value = data.name
        created_at.value = data.created_at
    }

    function clearRoom() {
        room_id.value = null
        name.value = null
        created_at.value = null
    }

    return { id, name, created_at, setRoom, clearRoom }
})
